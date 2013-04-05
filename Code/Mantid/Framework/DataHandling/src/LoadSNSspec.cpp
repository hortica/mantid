/*WIKI* 

The LoadSNSspec algorithm reads in spectra data from a text file and stores it in a Workspace2D as data points. The data in the file must be organized by set of 3 columns (separated by any number of spaces). The first column has to be the X values, the second column the Y values and the third column the error values. 

Here are two examples of such text files that can be loaded with LoadSNSspec:

''Example 1:''
<pre>
#F norm: REF_M_2000.nxs
#F data: REF_M_2001.nxs
#E 1234567.80
...
#C SCL Version - 1.4.1

#S 1 Spectrum ID ('bank1',(0,127))
#L Lambda_T(Angstroms) Intensity(Counts/A) Sigma(Counts/A)
0.0   0.0   0.0
1.0   5.0   2.0
2.0   10.0  3.0
3.0   15.0  2.0
4.0   20.0  2.5
5.0   25.0  3.2
6.0   30.0  4.2
</pre>
This will create a Workspace2D with 1 spectrum.

''Example 2:''
<pre>
#F norm: REF_M_2000.nxs
#F data: REF_M_2001.nxs
#E 1234567.80
...
#C SCL Version - 1.4.1

#S 1 Spectrum ID ('bank1',(0,127))
#L Lambda_T(Angstroms) Intensity(Counts/A) Sigma(Counts/A)
0.0   0.0   0.0
1.0   5.0   2.0
2.0   10.0  3.0
3.0   15.0  4.0

#S 1 Spectrum ID ('bank1',(1,127))
#L Lambda_T(Angstroms) Intensity(Counts/A) Sigma(Counts/A)
0.0   10.0   0.0
1.0   15.0   2.0
2.0   110.0  3.0
3.0   115.0  4.0

#S 1 Spectrum ID ('bank1',(3,127))
#L Lambda_T(Angstroms) Intensity(Counts/A) Sigma(Counts/A)
0.0   20.0   0.0
1.0   25.0   2.0
2.0   210.0  3.0
3.0   215.0  4.0
</pre>
This text file will create a Workspace2D with 3 spectra.

*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidDataHandling/LoadSNSspec.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/ListValidator.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/LoadAlgorithmFactory.h"
#include <fstream>
#include <cstring>
#include <boost/tokenizer.hpp>

namespace Mantid
{
  namespace DataHandling
  {
    // Register the algorithm into the algorithm factory
    DECLARE_ALGORITHM(LoadSNSspec)
      //register the algorithm into loadalgorithm factory
    DECLARE_LOADALGORITHM(LoadSNSspec)
    
    /// Sets documentation strings for this algorithm
    void LoadSNSspec::initDocs()
    {
      this->setWikiSummary("Loads data from a text file and stores it in a 2D [[workspace]] ([[Workspace2D]] class).");
      this->setOptionalMessage("Loads data from a text file and stores it in a 2D workspace (Workspace2D class).");
    }
    

    using namespace Kernel;
    using namespace API;

    /// Empty constructor
    LoadSNSspec::LoadSNSspec()  {}

    /// Initialisation method.
    void LoadSNSspec::init()
    {
      std::vector<std::string> exts;
	  exts.push_back(".dat");
	  exts.push_back(".txt");

	  declareProperty(new FileProperty("Filename", "", FileProperty::Load, exts),
			"The name of the text file to read, including its full or relative path. The file extension must be .txt or .dat.");
	  declareProperty(new WorkspaceProperty<>("OutputWorkspace",
			"",Direction::Output), "The name of the workspace that will be created, filled with the read-in data and stored in the [[Analysis Data Service]].");

	  std::vector<std::string> units = UnitFactory::Instance().getKeys();
	  declareProperty("Unit","Energy", boost::make_shared<Kernel::StringListValidator>(units),
			"The unit to assign to the X axis (anything known to the [[Unit Factory]] or \"Dimensionless\") (default: Energy)");
    }

    /**
    *   Executes the algorithm.
    */
    void LoadSNSspec::exec()
    {
      std::string filename = getProperty("Filename");
      std::ifstream file(filename.c_str());

      file.seekg (0, std::ios::end);
      Progress progress(this,0,1,static_cast<int>(file.tellg()));
      file.seekg (0, std::ios::beg);

      std::string str;

      std::vector<DataObjects::Histogram1D> spectra;
      //size_t iLine=0;    // line number
      int nSpectra = 0;
      int nBins = 0; //number of rows

      //bool numeric = true;
      std::vector<double> input;

      //determine the number of lines starting by #L
      //as there is one per set of data
      int spectra_nbr = 0;
      while(getline(file,str))
        {
          if (str.empty() ) continue;
          if (str[0] == '#' && str[1] == 'L')
            {
              spectra_nbr++;
            }
        }

      spectra.resize(spectra_nbr);
      file.clear(); //end of file has been reached so we need to clear file state
      file.seekg (0, std::ios::beg); //go back to beginning of file

      int working_with_spectrum_nbr = -1; //spectrum number
      while(getline(file,str))
        {
          progress.report(static_cast<int>(file.tellg()));

          //line with data, need to be parsed by white spaces
          if (!str.empty() && str[0] != '#')
            {
              typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
              boost::char_separator<char> sep(" ");
              tokenizer tok(str, sep);
              for (tokenizer::iterator beg=tok.begin(); beg!=tok.end(); ++beg)
                {
                  std::stringstream ss;
                  ss << *beg;
                  double d;
                  ss >> d;
                  input.push_back(d);
                }
            }

          if (str.empty())
            {
              if (working_with_spectrum_nbr != -1)
                {
                  for(int j=0; j<static_cast<int>(input.size()-1); j++)
                    {
                      spectra[working_with_spectrum_nbr].dataX().push_back(input[j]);
                      j++;
                      spectra[working_with_spectrum_nbr].dataY().push_back(input[j]);
                      j++;
                      spectra[working_with_spectrum_nbr].dataE().push_back(input[j]);
                      nBins = j/3;
                    }
                  spectra[working_with_spectrum_nbr].dataX().push_back(input[input.size()-1]);
                }
              working_with_spectrum_nbr++;
              input.clear();
            }

        } //end of read file

      try
      {
          if (spectra_nbr == 0)
            throw "Undefined number of spectra";

          if (working_with_spectrum_nbr == (spectra_nbr-1))
            {
              for(int j=0; j<static_cast<int>(input.size()-1); j++)
                {
                  spectra[working_with_spectrum_nbr].dataX().push_back(input[j]);
                  j++;
                  spectra[working_with_spectrum_nbr].dataY().push_back(input[j]);
                  j++;
                  spectra[working_with_spectrum_nbr].dataE().push_back(input[j]);
                  nBins = j/3;
                }
              spectra[working_with_spectrum_nbr].dataX().push_back(input[input.size()-1]);
            }
      }
      catch (...)
      {
      }

      try
      {
          nSpectra = spectra_nbr;
          MatrixWorkspace_sptr localWorkspace = boost::dynamic_pointer_cast<MatrixWorkspace>
          (WorkspaceFactory::Instance().create("Workspace2D",nSpectra,nBins+1,nBins));

          localWorkspace->getAxis(0)->unit() = UnitFactory::Instance().create(getProperty("Unit"));

          for(int i=0;i<nSpectra;i++)
            {
              localWorkspace->dataX(i) = spectra[i].dataX();
              localWorkspace->dataY(i) = spectra[i].dataY();
              localWorkspace->dataE(i) = spectra[i].dataE();
              // Just have spectrum number start at 1 and count up
              localWorkspace->getSpectrum(i)->setSpectrumNo(i+1);
            }

          setProperty("OutputWorkspace",localWorkspace);
      }
      catch (Exception::NotFoundError &)
      {
          // Asked for dimensionless workspace (obviously not in unit factory)
      }

    }
    /**This method does a quick file check by checking the no.of bytes read nread params and header buffer
 *  @param filePath- path of the file including name.
 *  @param nread :: no.of bytes read
 *  @param header :: The first 100 bytes of the file as a union
 *  @return true if the given file is of type which can be loaded by this algorithm
 */
    bool LoadSNSspec::quickFileCheck(const std::string& filePath,size_t nread,const file_header& header)
    {
      std::string extn=extension(filePath);
      bool bascii(false);
      (!extn.compare("txt"))?bascii=true:bascii=false;

      bool is_ascii (true);
      for(size_t i=0; i<nread; i++)
        {
          if (!isascii(header.full_hdr[i]))
            is_ascii =false;
        }
      return(is_ascii|| bascii?true:false);
    }

/**checks the file by opening it and reading few lines 
 *  @param filePath :: name of the file including its path
 *  @return an integer value how much this algorithm can load the file 
 */
    int LoadSNSspec::fileCheck(const std::string& filePath)
    {  
      std::ifstream file(filePath.c_str());
      if (!file)
      {
        g_log.error("Unable to open file: " + filePath);
        throw Exception::FileError("Unable to open file: " , filePath);
      }

      int bret=0;
      int axiscols=0;
      int datacols=0;
      std::string str;
      typedef boost::tokenizer<boost::char_separator<char> > tokenizer; 
      boost::char_separator<char> sep(" ");   
      bool bsnsspec(false);
      while(!file.eof())
      {
        //read line
        std::getline(file,str);
        if(str.empty())
        {
          continue;
        }
        try
        {
          //if it's comment line
          if (str.at(0)=='#' ) 
          {  
            if(str.at(1) =='L')  
            {
              tokenizer tok(str, sep); 
              for (tokenizer::iterator beg=tok.begin(); beg!=tok.end(); ++beg)
              {		 
                ++axiscols;
              }
              //if the file contains a comment line starting with "#L" followed
              //by three columns this could be loadsnsspec file
              if(axiscols>2)
              {                
                bsnsspec=true;
              }
            }
          }
          else 
          {  
            //check first data line is a 3 column line
            tokenizer tok(str, sep); 
            for (tokenizer::iterator beg=tok.begin(); beg!=tok.end(); ++beg)
            {		 
              ++datacols;
            } 
            break;
          }
        }
        catch(std::out_of_range& )
        {
        } 
      }
      if(bsnsspec && datacols==3 )//three column data
      {
        bret=80;
      }
      return bret;
    }
  } // namespace DataHandling
} // namespace Mantid
