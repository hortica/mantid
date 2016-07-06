#pylint: disable=no-init,invalid-name
from mantid.simpleapi import *
from mantid.kernel import StringListValidator, IntBoundedValidator, Direction
from mantid.api import DataProcessorAlgorithm, PropertyMode, AlgorithmFactory, \
                       MultipleFileProperty, FileProperty, FileAction, \
                       MatrixWorkspaceProperty, WorkspaceGroup
from mantid import config, logger, mtd

import numpy as np
import os.path

#pylint: disable=too-many-instance-attributes
class IndirectILLReduction(DataProcessorAlgorithm):

    #workspace names
    _raw_ws_name = None
    _grouped_ws_name = None
    _calib_ws_name = None
    _mnorm_ws_name = None
    _vnorm_ws_name = None
    _unmirror_ws_name = None
    _red_ws_name = None
    _monitor_ws_name = None

    #files names
    _run_file_name = None
    _map_file_name = None
    _parameter_file_name = None

    #bool flags
    _mirror_sense = None
    _control_mode = None
    _plot = None
    _save = None

    #other
    _analyser = None
    _instrument = None
    _instrument_name = None
    _reflection = None
    _unmirror_option = None

    def category(self):
        return "Workflow\\MIDAS;Inelastic\\Reduction"

    def summary(self):
        return 'Performs an energy transfer reduction for ILL indirect inelastic data.'

    def PyInit(self):
        # Input options
        self.declareProperty(MultipleFileProperty('Run', extensions=["nxs"]),
                                                  doc='File path of run (s).')

        self.declareProperty(name='Analyser', defaultValue='silicon',
                             validator=StringListValidator(['silicon']),
                             doc='Analyser crystal.')

        self.declareProperty(name='Reflection', defaultValue='111',
                             validator=StringListValidator(['111']),
                             doc='Analyser reflection.')

        self.declareProperty(FileProperty('MapFile', '',
                                          action=FileAction.OptionalLoad,
                                          extensions=["xml"]),
                             doc='Filename of the map file to use. If left blank the default will be used.')

        self.declareProperty(name='MirrorSense', defaultValue=True,
                             doc='Whether mirror sense is enabled.')

        self.declareProperty(name='ControlMode', defaultValue=False,
                             doc='Whether to output the workspaces in intermediate steps.')

        self.declareProperty(name='UnmirrorOption', defaultValue=7,
                             validator=IntBoundedValidator(lower=0,upper=7),
                             doc='Unmirroring option.')

        # Output workspace properties
        self.declareProperty(MatrixWorkspaceProperty("RawWorkspace", "raw",
                                                     direction=Direction.Output),
                             doc="Name for the output raw workspace created.")

        self.declareProperty(MatrixWorkspaceProperty("ReducedWorkspace", "red",
                                                     direction=Direction.Output),
                             doc="Name for the output reduced workspace created.")

        self.declareProperty(MatrixWorkspaceProperty("MNormalisedWorkspace", "mnorm",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Output),
                             doc="Name for the workspace normalised to monitor.")

        self.declareProperty(MatrixWorkspaceProperty("DetGroupedWorkspace", "grouped",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Output),
                             doc="Name for the workspace with grouped detectors.")

        self.declareProperty(MatrixWorkspaceProperty("MonitorWorkspace", "monitor",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Output),
                             doc="Name for the monitor spectrum.")

        self.declareProperty(MatrixWorkspaceProperty("VNormalisedWorkspace", "vnorm",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Output),
                             doc="Name for the workspace normalised to vanadium.")

        self.declareProperty(MatrixWorkspaceProperty("UnmirroredWorkspace", "unmirrored",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Output),
                             doc="Name for the unmirrored workspace.")

        # Optional input calibration workspace
        self.declareProperty(MatrixWorkspaceProperty("CalibrationWorkspace", "",
                                                     optional=PropertyMode.Optional,
                                                     direction=Direction.Input),
                             doc="Workspace containing calibration intensities.")

        # Output options
        self.declareProperty(name='Save', defaultValue=False,
                             doc='Whether to save the reduced workpsace to nxs file.')

        self.declareProperty(name='Plot', defaultValue=False,
                             doc='Whether to plot the reduced workspace.')

        self.validateInputs()

    def validateInputs(self):

        issues = dict()
        return issues

    def setUp(self):

        self._run_file_name = self.getPropertyValue('Run')
        self._calib_ws_name = self.getPropertyValue('CalibrationWorkspace')
        self._raw_ws_name = self.getPropertyValue('RawWorkspace')
        self._red_ws_name= self.getPropertyValue('ReducedWorkspace')
        self._vnorm_ws_name = self.getPropertyValue('VNormalisedWorkspace')
        self._mnorm_ws_name = self.getPropertyValue('MNormalisedWorkspace')
        self._unmirror_ws_name = self.getPropertyValue('UnmirroredWorkspace')
        self._grouped_ws_name = self.getPropertyValue('DetGroupedWorkspace')
        self._monitor_ws = self.getPropertyValue('MonitorWorkspace')
        self._map_file_name = self.getPropertyValue('MapFile')
        self._analyser = self.getPropertyValue('Analyser')
        self._reflection = self.getPropertyValue('Reflection')

        self._mirror_sense = self.getProperty('MirrorSense').value
        self._control_mode = self.getProperty('ControlMode').value
        self._unmirror_option = self.getProperty('UnmirrorOption').value
        self._save = self.getProperty('Save').value
        self._plot = self.getProperty('Plot').value

    def PyExec(self):

        self.setUp()

        Load(Filename=self._run_file_name,OutputWorkspace=self._raw_ws_name)

        self.log().information('Loaded .nxs file(s) : %s' % self._run_file_name)

        self.setPropertyValue('RawWorkspace', self._raw_ws_name)

        if isinstance(mtd[self._raw_ws_name],WorkspaceGroup):
            self._instrument = mtd[self._raw_ws_name].getItem(0).getInstrument()
            self._loadParamFiles()
            out_ws = self._multifile_reduction(mtd[self._raw_ws_name])
        else:
            self._instrument = mtd[self._raw_ws_name].getInstrument()
            self._loadParamFiles()
            out_ws = self._reduction(mtd[self._raw_ws_name], False)

        if not self._control_mode:
            self.setProperty('ReducedWorkspace', out_ws)
        else:
            self.setProperty('MonitorWorkspace', out_ws[0])
            self.setProperty('DetGroupedWorkspace', out_ws[1])
            self.setProperty('MNormalisedWorkspace', out_ws[2])
            self.setProperty('VNormalisedWorkspace', out_ws[3])
            self.setProperty('UnmirroredWorkspace', out_ws[4])
            self.setProperty('ReducedWorkspace', out_ws[5])

    def _loadParamFiles(self):
        """
        Loads parameter and detector grouping map file
        """

        self._instrument_name = self._instrument.getName()
        idf_directory = config['instrumentDefinition.directory']
        ipf_name = self._instrument_name + '_' + self._analyser + '_' + self._reflection + '_Parameters.xml'
        self._parameter_file_name = os.path.join(idf_directory, ipf_name)

        logger.information('Parameter file : %s' % self._parameter_file_name)

        if self._map_file_name == '':
            # path name for default map file
            if self._instrument.hasParameter('Workflow.GroupingFile'):
               grouping_filename = self._instrument.getStringParameter('Workflow.GroupingFile')[0]
               self._map_file_name = os.path.join(config['groupingFiles.directory'], grouping_filename)
            else:
              raise ValueError("Failed to find default detector grouping file. Please specify.")

        logger.information('Map file : %s' % self._map_file_name)

    def _save(self):
        """
        Saves the reduced workspace
        """

        workdir = config['defaultsave.directory']
        file_path = os.path.join(workdir, self._red_ws_name+ '.nxs')
        SaveNexusProcessed(InputWorkspace=self._red_ws_name, Filename=file_path)
        logger.information('Output file : ' + file_path)

    def _plot(self):
        """
        Plots the reduced workspace
        """

        from IndirectImport import import_mantidplot
        mtd_plot = import_mantidplot()
        graph = mtd_plot.newGraph()
        mtd_plot.plotSpectrum(self._red_ws_name, 0, window=graph)
        layer = graph.activeLayer()
        layer.setAxisTitle(mtd_plot.Layer.Bottom, 'Energy Transfer (micro eV)')
        layer.setAxisTitle(mtd_plot.Layer.Left, '')
        layer.setTitle('')

    def _multifile_reduction(self, gws):
        """
        Calls _reduction for GroupWorkspace

        @param gws :: input workspace group to reduce
        @return    :: the reduced group workspace or a list of group
                      workspaces depending on the _control_mode
        """

        list_red = []
        list_monitor = []
        list_grouped = []
        list_mnorm = []
        list_vnorm = []
        list_unmirrored = []

        size = gws.size()

        for i in range(0,size):
            out = self._reduction(gws.getItem(i),True)
            if not self._control_mode:
                list_red.append(out)
            else:
                list_monitor.append(out[0])
                list_grouped.append(out[1])
                list_mnorm.append(out[2])
                list_vnorm.append(out[3])
                list_unmirrored.append(out[4])
                list_red.append(out[5])

        group_red = GroupWorkspaces(list_red)

        if not self._control_mode:
            return group_red
        else:
            group_monitor = GroupWorkspaces(list_monitor)
            group_grouped = GroupWorkspaces(list_grouped)
            group_mnorm = GroupWorkspaces(list_mnorm)
            group_vnorm = GroupWorkspaces(list_vnorm)
            group_unmirrored = GroupWorkspaces(list_unmirrored)
            return [group_monitor,group_grouped,group_mnorm,
                    group_vnorm,group_unmirrored,group_red]

    def _reduction(self, ws, multiple = False):
        """
        Run indirect reduction for IN16B

        @param ws       :: name of the input raw workspace (single) to reduce
        @param multiple :: bool flag indicating if it is a part of multifile reduction
        @return         :: the reduced workspace or a list of workspaces
                           depending on the control mode
        """

        logger.information('Reducting input workspace : %s' % ws.name)

        if multiple:
           run_number = str(ws.getRunNumber()) + '_'
        else:
           run_number = ""

        monitor =  run_number + self._monitor_ws
        grouped =  run_number + self._grouped_ws_name
        mnorm = run_number + self._mnorm_ws_name
        vnorm =  run_number + self._vnorm_ws_name
        unmirror =  run_number + self._unmirror_ws_name
        red = run_number + self._red_ws_name

        LoadParameterFile(Workspace=ws, Filename=self._parameter_file_name)

        AddSampleLog(Workspace=ws,
                     LogName="Facility",
                     LogType="String",
                     LogText="ILL")

        ExtractSingleSpectrum(InputWorkspace=ws,
                              OutputWorkspace=monitor,
                              WorkspaceIndex=0)

        GroupDetectors(InputWorkspace=ws,
                       OutputWorkspace=grouped,
                       MapFile=self._map_file_name,
                       Behaviour='Average')

        NormaliseToMonitor(InputWorkspace = grouped,
                           MonitorWorkspace = monitor,
                           OutputWorkspace = mnorm)

        if self._calib_ws_name != '':
            vnorm = self._normalise_to_vanadium(mnorm, self._calib_ws_name)
        else:
            vnorm = mnorm

        if self._mirror_sense:
            unmirror = self._unmirror(vnorm)
        else:
            unmirror = vnorm

        red = self._calculate_energy(mnorm, red) #should be unmirror

        if not self._control_mode:
            DeleteWorkspace(monitor)
            DeleteWorkspace(mnorm)
            DeleteWorkspace(grouped)
            return red
        else:
            return [monitor,grouped,mnorm,
                    vnorm,unmirror,red]

    def _normalise_to_vanadium(self, ws, van):
        """
        Normalises to vanadium workspace.

        @param ws  :: name of the spectra to normalise
        @param van :: name of the vanadium reference workspace
        @return    :: the spectra normalised to vanadium
        """
        return ws

    def _unmirror(self, ws):
        """
        Sums up left and right wings of IN16B data.

        @param ws :: name of the input workspace containing two wings
        @return   :: unmirrored workspace
        """

        #switch over self._unmirror_option and do correspondingly

        return ws

    def _calculate_energy(self, ws, red):
        """
        Convert the input run to energy transfer
        @param ws :: name of the input workspace
        @param ws :: name of the output workspace
        @return   :: workspace with x-axis converted to energy transfer
        """

        formula = self._energy_range(ws)

        ConvertAxisByFormula(InputWorkspace=ws,
                             OutputWorkspace=red,
                             Axis='X',
                             Formula=formula)

        mtd[red].getAxis(0).setUnit('DeltaE')

        xnew = mtd[red].readX(0)  # energy array
        logger.information('Energy range : %f to %f' % (xnew[0], xnew[-1]))

        return mtd[red]

    def _energy_range(self, ws):
        """
        Calculate the energy range for the workspace

        @param ws :: name of the input workspace
        @return   :: formula to transform from time channel to energy transfer
        """
        x = mtd[ws].readX(0)
        npt = len(x)
        imid = float(npt / 2 + 1)
        gRun = mtd[ws].getRun()
        wave = gRun.getLogData('wavelength').value
        logger.information('Wavelength : %s' % wave)

        if gRun.hasProperty('Doppler.maximum_delta_energy'):
            energy = gRun.getLogData('Doppler.maximum_delta_energy').value
            logger.information('Doppler max energy : %s' % energy)

        dele = 2.0 * energy / npt
        formula = '(x-%f)*%f' % (imid, dele)

        return formula

# Register algorithm with Mantid
AlgorithmFactory.subscribe(IndirectILLReduction)
