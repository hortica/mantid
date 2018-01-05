import stresstesting

from mantid.simpleapi import PowderDiffILLDetScanReduction, \
    CompareWorkspaces, GroupWorkspaces
from mantid import config


class PowderDiffILLDetScanReductionTest(stresstesting.MantidStressTest):

    def __init__(self):
        super(PowderDiffILLDetScanReductionTest, self).__init__()
        self.setUp()

    def setUp(self):
        config['default.facility'] = 'ILL'
        config['default.instrument'] = 'D2B'
        config.appendDataSearchSubDir('ILL/D2B/')

    def requiredFiles(self):
        return ["508093.nxs, 508094.nxs, 508095.nxs, D2B_scan_test.nxs"]

    def d2b_2d_test(self):
        ws_2d_tubes = PowderDiffILLDetScanReduction(
            Run='508093:508095',
            Output2DTubes = True,
            Output2D = False,
            Output1D = False,
            OutputWorkspace='outWS_2d_tubes')
        return ws_2d_tubes

    def d2b_2d_test_using_merge(self):
        ws_2d_tubes_merge = PowderDiffILLDetScanReduction(
            Run='508093-508095',
            Output2DTubes = True,
            Output2D = False,
            Output1D = False,
            OutputWorkspace='outWS_2d_tubes_merge')
        return ws_2d_tubes_merge

    def d2b_2d_straight_test(self):
        ws_2d = PowderDiffILLDetScanReduction(
            Run = '508093:508095',
            UseCalibratedData = False,
            NormaliseTo = 'None',
            Output2DTubes = False,
            Output2D = True,
            Output1D = False,
            OutputWorkspace='outWS_2d_straight')
        return ws_2d

    def d2b_1d_test(self):
        ws_1d = PowderDiffILLDetScanReduction(
            Run='508093:508095',
            Output2DTubes = False,
            Output2D = False,
            Output1D = True,
            OutputWorkspace='outWS_1d')
        return ws_1d

    def runTest(self):
        ws_2d_tubes = self.d2b_2d_test()
        ws_2d = self.d2b_2d_straight_test()
        ws_1d = self.d2b_1d_test()

        # Check loading and merging, and keeping files separate gives the same results
        ws_2d_merge = self.d2b_2d_test_using_merge()
        result = CompareWorkspaces(Workspace1=ws_2d, Workspace2=ws_2d_merge)
        self.assertTrue(result)

        GroupWorkspaces([ws_2d_tubes[0], ws_2d[0], ws_1d[0]], OutputWorkspace='grouped_output')

    def validate(self):
        return 'grouped_output', 'D2B_scan_test.nxs'
