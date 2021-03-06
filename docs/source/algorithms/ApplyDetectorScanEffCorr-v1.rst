.. algorithm::

.. summary::

.. relatedalgorithms::

.. properties::

Description
-----------

This algorithm applies a calibration file to a workspace with a detector scan. This is currently used to support powder
diffractometers D2B and D20 at the ILL, but could be used for other detector scan instruments too. This can also be used
to apply a calibration to a non-detector scan instrument, but in this case the algorithm is just the multiplication of
the two workspaces.

The calibration file would normally be generated by using the algorithm
:ref:`PowderILLEfficiency <algm-PowderILLEfficiency>`. The structure is a single column with an entry for each
detector.

The data to apply the calibration to would normally be loaded by
:ref:`LoadILLDiffraction <algm-LoadILLDiffraction>`. The structure expected is a spectra for each time index for the
first detector, followed by a spectra for each time index for the next detector and so on. This includes the monitors,
which must be the first entries in the workspace, and which have no entries in the calibration file.

The calibration currently files have no errors associated, so the error in the input workspace is scaled by the
calibration factor.

The bins that are masked in the calibration workspace (i.e. constant is equal to 0) will be propagated to the output workspace.
That is, they will not only zero the counts of the corresponding pixel (multiplicative correction of 0), but they will also be flagged as masked.
This will then allow :ref:`SumOverlappingTubes <algm-SumOverlappingTubes>` to correctly treat those pixels as masked.

.. categories::

.. sourcelink::
