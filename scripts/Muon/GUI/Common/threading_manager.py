from __future__ import (absolute_import, division, print_function)

import traceback
import sys

from qtpy import QtCore, QtWidgets
from qtpy.QtCore import Signal


def split_list_into_n_parts(lst, n):
    return [lst[i::n] for i in range(n)]


def column(matrix, i):
    return [row[i] for row in matrix]


def split_kwarg_list(kwarg_list, n):
    split = lambda x: split_list_into_n_parts(x, n)
    chunks = list(map(split, kwarg_list.values()))
    return [dict(zip(kwarg_list.keys(), column(chunks, i))) for i in range(n)]


# Wraps a function, adding the following :
# - Converts function arguments to allow for lists of arguments to be passed with the same names.
# - Catches exceptions and returns a list of these as second return value.
# - Adds a new argument 'progress_callback', to which user passes a signal, this is then emitting with
# the progression of the evaluation of the argument list.
def threading_decorator(fn):
    def wrapper(progress_callback, **kwargs):
        results, failed_results_exceptions = [], []
        num_evals = len(kwargs.values()[0])
        input_list = [{key: value[i] for key, value in kwargs.items()} for i in range(num_evals)]
        for inputs in input_list:
            try:
                result = fn(**inputs)
            except Exception as e:
                failed_results_exceptions += [e]
                continue
            results += [result]
            progress_callback.emit(100 / num_evals)
        return results, failed_results_exceptions

    return wrapper


class WorkerSignals(QtCore.QObject):
    """
    Defines the signals available from a running worker thread.
    """
    started = Signal()
    finished = Signal()
    error = Signal(dict)
    result = Signal(tuple)
    progress = Signal(int)
    cancelled = Signal()


class Worker(QtCore.QThread):
    """
    Worker thread. Inherits from QThread to handle worker thread setup, signals and wrap-up. Using QThread here
    (instead of QRunnable) so we can cancel the thread.

    fn : A function to evaluate
    kwargs : keyword arguments to pass to fn

    returns are via signals defined in WorkerSignals()
    """

    def __init__(self, fn, **kwargs):
        """
        :param fn: The function to be evaluated.
        :param kwargs: Keyword argument lists to pass to the function (e.g. x =[1,2,3] for f(x) where x is an integer).
        """
        super(Worker, self).__init__()
        self.fn = threading_decorator(fn)
        self.kwargs = kwargs
        self.signals = WorkerSignals()
        # Add callback so function can update its progress
        self.kwargs['progress_callback'] = self.signals.progress

    def run(self):
        self.signals.started.emit()
        try:
            result, fails = self.fn(**self.kwargs)
            if 'progress_callback' in self.kwargs: del self.kwargs['progress_callback']
            for fail in fails:
                # Exceptions caught whilst executing the function
                exctype, value = type(fail), next(iter(fail.args), "")
                self.signals.error.emit({"inputs": self.kwargs,
                                         "exctype": exctype,
                                         "value": value,
                                         "traceback": traceback.format_exc()})
        except:
            # Catch errors not thrown by the function evaluation
            exctype, value = sys.exc_info()[:2]  # traceback.print_exc()
            if 'progress_callback' in self.kwargs: del self.kwargs['progress_callback']
            self.signals.error.emit({"inputs": self.kwargs,
                                     "exctype": exctype,
                                     "value": value,
                                     "traceback": traceback.format_exc()})
        else:
            # Successful evaluation
            if 'progress_callback' in self.kwargs: del self.kwargs['progress_callback']
            self.signals.result.emit((self.kwargs, result))
        finally:
            self.signals.finished.emit()

    def cancel(self):
        self.signals.cancelled.emit()
        # must use terminate (not quit/exit) as there is no event loop for the thread
        self.terminate()


class WorkerManager(QtWidgets.QWidget):
    """
    The WorkerManager class handles multi-threading of a function, by splitting an arbitrary number of list arguments
    equally across a specified number of threads.

    Example : for a function f(x,y) where x and y are ints, the WorkerManager allows for evaluation of
    f(x=[1,2,3,4,5],y=[1,2,3,4,5]) by splitting the lists across the given number of threads. So for two threads the
    following evaluations might occur:
        Thread 1 : f(1,1), f(3,3), f(5,5)
        Thread 2 : f(2,2), f(4,4)

    A list of callbacks can be passed into the constructor to respond to events (all threads finishing,
    threads updating on progress, threads catching exceptions from the function evaluation)
    """

    finished = Signal()  # used for unit tests
    cancelled = Signal()  # used for unit tests

    def __init__(self, fn, num_threads,
                 callback_on_threads_complete=lambda: 0,
                 callback_on_progress_update=lambda dbl: 0,
                 callback_on_thread_exception=lambda err_dict: 0,
                 verbose=0,
                 **kwarg_list):
        """

        :param fn: The function to be evaluated.
        :param num_threads: Number of threads to use.
        :param callback_on_threads_complete: Optional callable for when all threads are finished
        :param callback_on_progress_update: Optional callback for progress updates
        :param callback_on_thread_exception: Optional callback called on exception from thread
        :param verbose: 0,1 whether to print output
        :param kwarg_list: Keyword argument lists to apply to function (e.g. x =[1,2,3] for f(x) where x is an integer)
        """
        super(WorkerManager, self).__init__()

        # Callback called when all threads have executed
        self.thread_complete_callback = callback_on_threads_complete
        # Callback called when a single thread updates its progress
        self.progress_callback = callback_on_progress_update
        # Callback called when an error is thrown from a thread
        self.error_callback = callback_on_thread_exception

        self.fn = fn
        self.kwarg_list = kwarg_list

        # maintain a list of running threads to allow for cancelling
        self._threads = []

        self._results = {key: [] for key in self.kwarg_list.keys() + ['results']}
        self._failed_results = {key: [] for key in self.kwarg_list.keys()}

        self._progress = 0.0

        self._num_threads = num_threads  # total number of threads started
        self._thread_count = 0  # current threads running

        self.mutex = QtCore.QMutex()

        self.verbose = verbose

    def _cancel_threads(self):
        for thread in self._threads:
            if thread.isRunning():
                thread.cancel()

    def _clear_threads(self):
        self._cancel_threads()
        self._threads = []
        self._thread_count = 0

    def _clear_results(self):
        self._progress = 0.0
        self._results = {key: [] for key in self.kwarg_list.keys() + ['results']}
        self._failed_results = {key: [] for key in self.kwarg_list.keys()}

    def cancel(self):
        self._clear_threads()
        self.cancelled.emit()  # for unit tests

    def clear(self):
        self._clear_threads()
        self._clear_results()

    def is_running(self):
        return self._thread_count > 0

    def start(self):
        if not self.is_running():
            self.clear()
            self._thread_count = self._num_threads
            self.on_thread_progress(0.0)

            thread_list = split_kwarg_list(self.kwarg_list, self._num_threads)
            for i, arg in enumerate(thread_list):
                worker = Worker(self.fn, **arg)
                self.connect_worker_signals(worker)
                self._threads += [worker]
                if self.verbose:
                    print("\tStarting thread " + str(i + 1) + " with arg : ", arg)
                worker.start()
        else:
            raise RuntimeError("Cannot start threads, "
                               "WorkerManager has active threads (call cancel() or clear() first)")

    def connect_worker_signals(self, worker):
        worker.signals.started.connect(self.on_thread_start)
        worker.signals.result.connect(self.on_thread_result)
        worker.signals.finished.connect(self.on_thread_complete)
        worker.signals.progress.connect(self.on_thread_progress)
        worker.signals.error.connect(self.on_thread_exception)
        worker.signals.cancelled.connect(self.on_thread_cancelled)

    def on_thread_start(self):
        pass

    def on_thread_cancelled(self):
        pass

    def on_thread_exception(self, kwargs):
        self.error_callback(**kwargs)

        self.mutex.lock()
        function_inputs = kwargs["inputs"]
        for key, item in function_inputs.items():
            self._failed_results[key] += item
        self.mutex.unlock()

    def on_thread_result(self, s):
        self.mutex.lock()
        input_kwargs, results = s
        for input_name, input_value in input_kwargs.items():
            self._results[input_name] += input_value
        self._results['results'] += results
        self.mutex.unlock()

    def on_thread_complete(self):
        if self.verbose:
            print("\tTHREAD " + str(self._thread_count) + " COMPLETE!\n")
        self.mutex.lock()
        self._thread_count -= 1
        self.mutex.unlock()

        if self._thread_count == 0:
            self.on_final_thread_complete()

    def on_final_thread_complete(self):
        self._cancel_threads()
        if self._progress < 100.0:
            # in case of exceptions in threads
            self._update_progress(100.0 - self._progress)
        self.thread_complete_callback()
        self.finished.emit()  # for unit tests

    def _update_progress(self, percentage):
        self._progress += percentage
        self.progress_callback(self._progress)

    def on_thread_progress(self, percentage):
        self.mutex.lock()
        self._update_progress(percentage * 1 / self._num_threads)
        self.mutex.unlock()

    @property
    def results(self):
        return self._results
