#ifndef BACKTOBACKEXPONENTIALTEST_H_
#define BACKTOBACKEXPONENTIALTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/BackToBackExponential.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/SpectraDetectorMap.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidDataHandling/LoadRaw.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using Mantid::CurveFitting::BackToBackExponential;
using namespace Mantid::DataObjects;
using namespace Mantid::DataHandling;

class BackToBackExponentialTest : public CxxTest::TestSuite
{
public:

  BackToBackExponentialTest()
  {
    std::string inputFile = "../../../../Test/Data/HRP38692.RAW";

    LoadRaw loader;

    loader.initialize();

    loader.setPropertyValue("Filename", inputFile);

    outputSpace = "B2BOuter";
    loader.setPropertyValue("OutputWorkspace", outputSpace);

    loader.execute();
  }

  void testInit()
  {
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized() );

    // Set the properties
    alg.setPropertyValue("InputWorkspace",outputSpace);
    alg.setPropertyValue("SpectrumIndex","2");
    alg.setPropertyValue("StartX","79280"); 
    alg.setPropertyValue("EndX","79615");   

    alg.setPropertyValue("I", "297.0");
    alg.setPropertyValue("a", "2.0");
    alg.setPropertyValue("b", "0.03");
    alg.setPropertyValue("x0", "79400.0");
    alg.setPropertyValue("s", "8.0");
    alg.setPropertyValue("bk", "8.0");
  }

  void testExec()
  {
    if ( !alg.isInitialized() ) alg.initialize();
    TS_ASSERT_THROWS_NOTHING( alg.execute());
    TS_ASSERT( alg.isExecuted() );

    double dummy = alg.getProperty("Output Chi^2/DoF");
    TS_ASSERT_DELTA( dummy, 9.02,0.1);
    dummy = alg.getProperty("I");
    TS_ASSERT_DELTA( dummy, 294.72 ,0.1);
    dummy = alg.getProperty("a");
    TS_ASSERT_DELTA( dummy, 2.38 ,0.1);
    dummy = alg.getProperty("b");
    TS_ASSERT_DELTA( dummy, 0.03 ,0.1);
    dummy = alg.getProperty("x0");
    TS_ASSERT_DELTA( dummy, 79400.02 ,0.1);
    dummy = alg.getProperty("s");
    TS_ASSERT_DELTA( dummy, 8.15 ,0.1);
    dummy = alg.getProperty("bk");
    TS_ASSERT_DELTA( dummy, 7.88 ,0.1);
  }


private:
  BackToBackExponential alg;
  std::string inputSpace;
  std::string outputSpace;
};

#endif /*BACKTOBACKEXPONENTIALTEST_H_*/
