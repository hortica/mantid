#ifndef MANTID_CURVEFITTING_USERFUNCTION1D_H_
#define MANTID_CURVEFITTING_USERFUNCTION1D_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/Fit1D.h"
#include <muParser.h>
#include <boost/shared_array.hpp>

namespace Mantid
{
  namespace CurveFitting
  {
    /**
    Fits a histogram in a 2D workspace to a user defined function.

    Properties:
    <UL>
    <LI> InputWorkspace - The name of the Workspace to take as input </LI>

    <LI> SpectrumIndex - The spectrum to fit, using the workspace numbering of the spectra (default 0)</LI>
    <LI> StartX - X value to start fitting from (default start of the spectrum)</LI>
    <LI> EndX - last X value to include in fitting range (default end of the spectrum)</LI>
    <LI> MaxIterations - Max iterations (default 500)</LI>
    <LI> Output Status - whether the fit was successful. Direction::Output</LI>
    <LI> Output Chi^2/DoF - returns how good the fit was (default 0.0). Direction::Output</LI>

    <LI> Function - The user defined function. It must have x as its argument.</LI>
    <LI> InitialParameters - A list of initial values for the parameters in the function. It is a comma separated
            list of name=value items where name is the name of a parameter and value is its initial vlaue.
            The name=value pairs can appear in any order in the list. The parameters that are not set in this property
            will be given the default initial value of 0.0</LI>
    <LI> Parameters - The output table workspace with the final values of the fit parameters</LI>
    <LI> OutputWorkspace - A matrix workspace to hold the resulting model spectrum, the initial histogram and the difference
            between them</LI>
    </UL>

    @author Roman Tolchenov, Tessella plc
    @date 17/6/2009

    Copyright &copy; 2007-8 STFC Rutherford Appleton Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
      class UserFunction1D : public Fit1D
      {
      public:
          /// Constructor
          UserFunction1D():m_x_set(false),m_parameters(new double[100]),m_buffSize(100),m_nPars(0) {};
          /// Destructor
          virtual ~UserFunction1D() {};
          /// Algorithm's name for identification overriding a virtual method
          virtual const std::string name() const { return "UserFunction1D";}
          /// Algorithm's version for identification overriding a virtual method
          virtual const int version() const { return (1);}
          /// Algorithm's category for identification overriding a virtual method
          virtual const std::string category() const { return "CurveFitting";}


      protected:
          /// overwrite base class methods
          double function(const double* in, const double& x);
          virtual void function(const double* in, double* out, const double* xValues, const double* yValues, const double* yErrors, const int& nData);
          virtual void declareAdditionalProperties();
          virtual void declareParameters(){};
          virtual void prepare();

          static double* AddVariable(const char *varName, void *palg);

      private:
          /// muParser instance
          mu::Parser m_parser;
          /// Used as 'x' variable in m_parser.
          double m_x;
          /// True indicates that input formula contains 'x' variable
          bool m_x_set;
          /// Pointer to muParser variables' buffer
          boost::shared_array<double> m_parameters;
          /// Size of the variables' buffer
          const int m_buffSize;
          /// Number of actual parameters
          int m_nPars;
      };

  } // namespace CurveFitting
} // namespace Mantid

#endif /*MANTID_CURVEFITTING_GAUSSIAN1D_H_*/
