/*WIKI*
Base class for other algorithms.
*WIKI*/

#include "MantidMDAlgorithms/BinaryOperationMD.h"
#include "MantidKernel/System.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidDataObjects/WorkspaceSingleValue.h"
#include "MantidMDEvents/MDHistoWorkspace.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceProperty.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::MDEvents;
using namespace Mantid::DataObjects;

namespace Mantid
{
namespace MDAlgorithms
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  BinaryOperationMD::BinaryOperationMD()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  BinaryOperationMD::~BinaryOperationMD()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string BinaryOperationMD::name() const { return "BinaryOperationMD";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int BinaryOperationMD::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string BinaryOperationMD::category() const { return "Arithmetic";}

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void BinaryOperationMD::init()
  {
    declareProperty(new WorkspaceProperty<IMDWorkspace>(inputPropName1(),"",Direction::Input),
        "An MDEventWorkspace, MDHistoWorkspace or WorkspaceSingleValue as the left-hand side of the operation.");
    declareProperty(new WorkspaceProperty<IMDWorkspace>(inputPropName2(),"",Direction::Input),
        "An MDEventWorkspace, MDHistoWorkspace or WorkspaceSingleValue as the right-hand side of the operation.");
    declareProperty(new WorkspaceProperty<IMDWorkspace>(outputPropName(),"",Direction::Output),
        "Name of the output MDEventWorkspace or MDHistoWorkspace.");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void BinaryOperationMD::exec()
  {
    // Get the properties
    m_lhs = getProperty( inputPropName1() );
    m_rhs = getProperty( inputPropName2() );
    m_out = getProperty( outputPropName() );

    // Flip LHS and RHS if commutative and :
    //  1. A = B + A -> becomes -> A += B
    //  1. C = 1 + A -> becomes -> C = A + 1   (number is always on RHS if possible)
    if (this->commutative() && ((m_out == m_rhs) ||
            boost::dynamic_pointer_cast<WorkspaceSingleValue>(m_lhs) ))
    {
      //
      // So we flip RHS/LHS
      Mantid::API::IMDWorkspace_sptr temp = m_lhs;
      m_lhs = m_rhs;
      m_rhs = temp;
    }

    // Can't do A = 1 / B
    if (boost::dynamic_pointer_cast<MatrixWorkspace>(m_lhs))
      throw std::invalid_argument("BinaryOperationMD: can't have a MatrixWorkspace (e.g. WorkspaceSingleValue) as the LHS argument of " + this->name() + ".");

    // Check the inputs. First cast to everything
    m_lhs_event = boost::dynamic_pointer_cast<IMDEventWorkspace>(m_lhs);
    m_lhs_histo = boost::dynamic_pointer_cast<MDHistoWorkspace>(m_lhs);
    m_rhs_event = boost::dynamic_pointer_cast<IMDEventWorkspace>(m_rhs);
    m_rhs_histo = boost::dynamic_pointer_cast<MDHistoWorkspace>(m_rhs);
    this->checkInputs();

    if (m_out == m_lhs)
    {
      // A = A * B. -> we will do A *= B
    }
    else
    {
      // C = A + B. -> So first we clone A (lhs) into C
      IAlgorithm_sptr clone = this->createSubAlgorithm("CloneMDWorkspace", 0.0, 0.5, true);
      clone->setProperty("InputWorkspace", m_lhs);
      clone->setPropertyValue("OutputWorkspace", getPropertyValue("OutputWorkspace"));
      clone->executeAsSubAlg();
      m_out = clone->getProperty("OutputWorkspace");
    }

    // Okay, at this point we are ready to do, e.g.,
    //  "m_out /= m_rhs"
    if (!m_out)
      throw std::runtime_error("Error creating the output workspace");
    if (!m_rhs)
      throw std::runtime_error("No RHS workspace specified!");

    m_operand_event = boost::dynamic_pointer_cast<IMDEventWorkspace>(m_rhs);
    m_operand_histo = boost::dynamic_pointer_cast<MDHistoWorkspace>(m_rhs);
    m_operand_scalar = boost::dynamic_pointer_cast<WorkspaceSingleValue>(m_rhs);

    m_out_event = boost::dynamic_pointer_cast<IMDEventWorkspace>(m_out);
    m_out_histo = boost::dynamic_pointer_cast<MDHistoWorkspace>(m_out);

    if (m_out_event)
    {
      // Call the templated virtual function for this type of MDEventWorkspace
      this->execEvent();
    }
    else if (m_out_histo)
    {
      // MDHistoWorkspace as the output
      if (m_operand_histo)
      {
        if (m_out_histo->getNumDims() != m_operand_histo->getNumDims())
          throw std::invalid_argument("Cannot perform " + this->name() + " on MDHistoWorkspace's with a different number of dimensions.");
        if (m_out_histo->getNPoints() != m_operand_histo->getNPoints())
          throw std::invalid_argument("Cannot perform " + this->name() + " on MDHistoWorkspace's with a different number of points.");
        this->execHistoHisto(m_out_histo, m_operand_histo);
      }
      else if (m_operand_scalar)
        this->execHistoScalar(m_out_histo, m_operand_scalar);
      else
        throw std::runtime_error("Unexpected operand workspace type. Expected MDHistoWorkspace or "
            "WorkspaceSingleValue, got " + m_rhs->id());
    }
    else
    {
      throw std::runtime_error("Unexpected output workspace type. Expected MDEventWorkspace or "
          "MDHistoWorkspace, got " + m_out->id());
    }

    // Give the output
    setProperty("OutputWorkspace", m_out);

  }



} // namespace Mantid
} // namespace MDAlgorithms
