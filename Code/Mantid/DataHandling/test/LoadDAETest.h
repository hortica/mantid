#ifndef LOADDAETEST_H_
#define LOADDAETEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidDataHandling/LoadDAE.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidDataObjects/ManagedWorkspace2D.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include "MantidAPI/SpectraDetectorMap.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::DataHandling;
using namespace Mantid::DataObjects;

class LoadDAETest : public CxxTest::TestSuite
{
public:

  LoadDAETest()
  {
    // Hostname of computer with DAE to connect to
    //m_inputDAE = "130.246.53.71";
	  m_inputDAE="ndw714";
  }

  void testInit()
  {
    TS_ASSERT_THROWS_NOTHING( m_loader.initialize());
    TS_ASSERT( m_loader.isInitialized() );
  }

  void testExec()
  {

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

	    if ( !m_loader.isInitialized() ) m_loader.initialize();

    // Should fail because mandatory parameter has not been set
   // TS_ASSERT_THROWS(m_loader.execute(),std::runtime_error);

    // Set inputs
    m_loader.setPropertyValue("DAEname", m_inputDAE);
    m_loader.setPropertyValue("SpectrumMin", "1");
    m_loader.setPropertyValue("SpectrumMax", "2");

    m_outputSpace = "DAEouter";
    m_loader.setPropertyValue("OutputWorkspace", m_outputSpace);

    std::string result;
    TS_ASSERT_THROWS_NOTHING( result = m_loader.getPropertyValue("DAEname") )
      TS_ASSERT( ! result.compare(m_inputDAE));

    TS_ASSERT_THROWS_NOTHING(m_loader.execute());
    TS_ASSERT_EQUALS( m_loader.isExecuted(),true);

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(m_outputSpace));
    if (output != 0)
    {
      Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

      // As we are checking a live DAE, we cannot be sure what we will see
      // as setup will change with experiments. All we can do is test
      // things that must always be true irrespective of setup

      // check number of spectra returned
      TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 2);

      // Check two X vectors are the same
      TS_ASSERT( (output2D->dataX(0)) == (output2D->dataX(1)) );

      // Check two Y arrays have the same number of elements
      TS_ASSERT_EQUALS( output2D->dataY(0).size(), output2D->dataY(1).size() );

      // Check the unit has been set correctly
      TS_ASSERT_EQUALS( output2D->getAxis(0)->unit()->unitID(), "TOF" )
        TS_ASSERT( ! output2D-> isDistribution() )
    }
  }
//  void testExecMultiPeriod()
//  {
//
//#ifdef _WIN32
//    WSADATA wsaData;
//    WSAStartup(MAKEWORD(2,2), &wsaData);
//#endif
//
//	    if ( !m_loader.isInitialized() ) m_loader.initialize();
//
//    // Should fail because mandatory parameter has not been set
//   // TS_ASSERT_THROWS(m_loader.execute(),std::runtime_error);
//
//	m_inputDAE="ndxcrisp";
//    // Set inputs
//    m_loader.setPropertyValue("DAEname", m_inputDAE);
//    m_loader.setPropertyValue("SpectrumMin", "1");
//    m_loader.setPropertyValue("SpectrumMax", "2");
//
//    m_outputSpace = "DAEouter";
//    m_loader.setPropertyValue("OutputWorkspace", m_outputSpace);
//
//    std::string result;
//    TS_ASSERT_THROWS_NOTHING( result = m_loader.getPropertyValue("DAEname") )
//    TS_ASSERT( ! result.compare(m_inputDAE));
//
//    TS_ASSERT_THROWS_NOTHING(m_loader.execute());
//    TS_ASSERT_EQUALS( m_loader.isExecuted(),true);
//
//	WorkspaceGroup_sptr outgrp;
//	 TS_ASSERT_THROWS_NOTHING(outgrp =boost::dynamic_pointer_cast<WorkspaceGroup> (AnalysisDataService::Instance().retrieve(m_outputSpace)));
//    // Get back the saved workspace
//    Workspace_sptr output;
//    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(m_outputSpace+"_1"));
//    if (output != 0)
//    {
//      Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);
//
//      // As we are checking a live DAE, we cannot be sure what we will see
//      // as setup will change with experiments. All we can do is test
//      // things that must always be true irrespective of setup
//
//      // check number of spectra returned
//      TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 4);
//
//      // Check two X vectors are the same
//      TS_ASSERT( (output2D->dataX(0)) == (output2D->dataX(1)) );
//
//      // Check two Y arrays have the same number of elements
//      TS_ASSERT_EQUALS( output2D->dataY(0).size(), output2D->dataY(1).size() );
//
//      // Check the unit has been set correctly
//      TS_ASSERT_EQUALS( output2D->getAxis(0)->unit()->unitID(), "TOF" )
//        TS_ASSERT( ! output2D-> isDistribution() )
//    }
//	 Workspace_sptr output2;
//     TS_ASSERT_THROWS_NOTHING(output2 = AnalysisDataService::Instance().retrieve(m_outputSpace+"_2"));
//  }

private:
  LoadDAE m_loader;
  std::string m_inputDAE;
  std::string m_outputSpace;
};

#endif /*LOADDAETEST_H_*/
