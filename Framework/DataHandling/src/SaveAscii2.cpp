//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include <set>
#include <fstream>

#include "MantidDataHandling/SaveAscii2.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidGeometry/Instrument.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/UnitConversion.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/VectorHelper.h"
#include "MantidKernel/VisibleWhenProperty.h"

#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

namespace Mantid {
namespace DataHandling {
// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(SaveAscii2)

using namespace Kernel;
using namespace API;

/// Empty constructor
SaveAscii2::SaveAscii2()
    : m_separatorIndex(), m_nBins(0), m_sep(), m_writeDX(false),
      m_writeID(false), m_isHistogram(false), m_isCommonBins(false), m_ws() {}

/// Initialisation method.
void SaveAscii2::init() {
  declareProperty(
      make_unique<WorkspaceProperty<>>("InputWorkspace", "", Direction::Input),
      "The name of the workspace containing the data you want to save to a "
      "Ascii file.");

  const std::vector<std::string> asciiExts{".dat", ".txt", ".csv"};
  declareProperty(Kernel::make_unique<FileProperty>(
                      "Filename", "", FileProperty::Save, asciiExts),
                  "The filename of the output Ascii file.");

  auto mustBePositive = boost::make_shared<BoundedValidator<int>>();
  mustBePositive->setLower(1);
  auto mustBeZeroGreater = boost::make_shared<BoundedValidator<int>>();
  mustBeZeroGreater->setLower(0);
  declareProperty("WorkspaceIndexMin", EMPTY_INT(), mustBeZeroGreater,
                  "The starting workspace index.");
  declareProperty("WorkspaceIndexMax", EMPTY_INT(), mustBeZeroGreater,
                  "The ending workspace index.");
  declareProperty(make_unique<ArrayProperty<int>>("SpectrumList"),
                  "List of workspace indices to save.");
  declareProperty("Precision", EMPTY_INT(), mustBePositive,
                  "Precision of output double values.");
  declareProperty("ScientificFormat", false, "If true, the values will be "
                                             "written to the file in "
                                             "scientific notation.");
  declareProperty(
      "WriteXError", false,
      "If true, the error on X will be written as the fourth column.");
  declareProperty("WriteSpectrumID", true,
                  "If false, the spectrum ID will not be written for "
                  "single-spectrum workspaces. "
                  "It is always written for workspaces with multiple spectra.");

  declareProperty("CommentIndicator", "#",
                  "Character(s) to put in front of comment lines.");

  // For the ListValidator
  std::string spacers[6][2] = {
      {"CSV", ","},   {"Tab", "\t"},      {"Space", " "},
      {"Colon", ":"}, {"SemiColon", ";"}, {"UserDefined", "UserDefined"}};
  std::vector<std::string> sepOptions;
  for (auto &spacer : spacers) {
    std::string option = spacer[0];
    m_separatorIndex.insert(
        std::pair<std::string, std::string>(option, spacer[1]));
    sepOptions.push_back(option);
  }

  declareProperty("Separator", "CSV",
                  boost::make_shared<StringListValidator>(sepOptions),
                  "The separator between data columns in the data file. The "
                  "possible values are \"CSV\", \"Tab\", "
                  "\"Space\", \"SemiColon\", \"Colon\" or \"UserDefined\".");

  declareProperty(
      make_unique<PropertyWithValue<std::string>>("CustomSeparator", "",
                                                  Direction::Input),
      "If present, will override any specified choice given to Separator.");

  setPropertySettings("CustomSeparator",
                      make_unique<VisibleWhenProperty>("Separator", IS_EQUAL_TO,
                                                       "UserDefined"));

  declareProperty("ColumnHeader", true,
                  "If true, put column headers into file. ");

  declareProperty("SpectrumMetaData", "",
                  "A comma separated list that defines data that describes "
                  "each spectrum in a workspace. The valid options for this "
                  "are: SpectrumNumber,Q,Angle");

  declareProperty(
      "AppendToFile", false,
      "If true, don't overwrite the file. Append to the end of it. ");

  declareProperty(
      "RaggedWorkspace", true,
      "If true, ensure that more than one xspectra is used. "); // in testing
}

/**
*   Executes the algorithm.
*/
void SaveAscii2::exec() {
  // Get the workspace
  m_ws = getProperty("InputWorkspace");
  m_specToIndexMap = m_ws->getSpectrumToWorkspaceIndexMap();
  int nSpectra = static_cast<int>(m_ws->getNumberHistograms());
  m_nBins = static_cast<int>(m_ws->blocksize());
  m_isHistogram = m_ws->isHistogramData();
  m_isCommonBins = m_ws->isCommonBins(); // checking for ragged workspace
  m_writeID = getProperty("WriteSpectrumID");
  const auto metaDataString = getPropertyValue("SpectrumMetaData");
  if (metaDataString.size() != 0) {
    m_metaData = stringListToVector(metaDataString);
  }
  if (m_writeID) {
    auto containsSpectrumNumber = false;
    for (auto iter = m_metaData.begin(); iter != m_metaData.end(); ++iter) {
      const auto metaDataType = *iter;
      if (metaDataType.compare("SpectrumNumber") == 0) {
        containsSpectrumNumber = true;
      }
    }
    if (containsSpectrumNumber == false) {
      auto firstIter = m_metaData.begin();
      m_metaData.insert(firstIter, "SpectrumNumber");
    }
  }

  // Get the properties
  std::vector<int> spec_list = getProperty("SpectrumList");
  const int spec_min = getProperty("WorkspaceIndexMin");
  const int spec_max = getProperty("WorkspaceIndexMax");
  const bool writeHeader = getProperty("ColumnHeader");
  const bool appendToFile = getProperty("AppendToFile");

  // Check whether we need to write the fourth column
  m_writeDX = getProperty("WriteXError");
  const std::string choice = getPropertyValue("Separator");
  const std::string custom = getPropertyValue("CustomSeparator");
  // If the custom separator property is not empty, then we use that under any
  // circumstance.
  if (custom != "") {
    m_sep = custom;
  }
  // Else if the separator drop down choice is not UserDefined then we use that.
  else if (choice != "UserDefined") {
    auto it = m_separatorIndex.find(choice);
    m_sep = it->second;
  }
  // If we still have nothing, then we are forced to use a default.
  if (m_sep.empty()) {
    g_log.notice() << "\"UserDefined\" has been selected, but no custom "
                      "separator has been entered."
                      " Using default instead.";
    m_sep = ",";
  }

  // e + and - are included as they're part of the scientific notation
  if (!boost::regex_match(m_sep.begin(), m_sep.end(),
                          boost::regex("[^0-9e+-]+", boost::regex::perl))) {
    throw std::invalid_argument("Separators cannot contain numeric characters, "
                                "plus signs, hyphens or 'e'");
  }

  std::string comment = getPropertyValue("CommentIndicator");

  if (comment.at(0) == m_sep.at(0) ||
      !boost::regex_match(
          comment.begin(), comment.end(),
          boost::regex("[^0-9e" + m_sep + "+-]+", boost::regex::perl))) {
    throw std::invalid_argument("Comment markers cannot contain numeric "
                                "characters, plus signs, hyphens,"
                                " 'e' or the selected separator character");
  }

  // Create an spectra index list for output
  std::set<int> idx;

  // Add spectra interval into the index list
  if (spec_max != EMPTY_INT() && spec_min != EMPTY_INT()) {
    if (spec_min >= nSpectra || spec_max >= nSpectra || spec_min < 0 ||
        spec_max < 0 || spec_min > spec_max) {
      throw std::invalid_argument("Inconsistent spectra interval");
    }
    for (int i = spec_min; i <= spec_max; i++) {
      idx.insert(i);
    }
  }
  // figure out how to read in readX and have them be seperate lists

  // Add spectra list into the index list
  if (!spec_list.empty()) {
    for (auto &spec : spec_list) {
      if (spec >= nSpectra) {
        throw std::invalid_argument("Inconsistent spectra list");
      } else {
        idx.insert(spec);
      }
    }
  }
  if (!idx.empty()) {
    nSpectra = static_cast<int>(idx.size());
  }

  if (m_nBins == 0 || nSpectra == 0) {
    throw std::runtime_error("Trying to save an empty workspace");
  }
  std::string filename = getProperty("Filename");
  std::ofstream file(filename.c_str(),
                     (appendToFile ? std::ios::app : std::ios::out));

  if (!file) {
    g_log.error("Unable to create file: " + filename);
    throw Exception::FileError("Unable to create file: ", filename);
  }
  // Set the number precision
  int prec = getProperty("Precision");
  if (prec != EMPTY_INT()) {
    file.precision(prec);
  }
  bool scientific = getProperty("ScientificFormat");
  if (scientific) {
    file << std::scientific;
  }
  if (writeHeader) {
    file << comment << " X " << m_sep << " Y " << m_sep << " E";
    if (m_writeDX) {
      file << " " << m_sep << " DX";
    }
    file << std::endl;
  }
  // populate the meta data map
  if (m_metaData.size() > 0) {
    populateAllMetaData();
  }
  if (idx.empty()) {
    Progress progress(this, 0, 1, nSpectra);
    for (int i = 0; i < nSpectra; i++) {
      writeSpectra(i, file);
      progress.report();
    }
  } else {
    Progress progress(this, 0, 1, idx.size());
    for (auto i = idx.begin(); i != idx.end(); ++i) {
      writeSpectra(i, file);
      progress.report();
    }
  }

  file.unsetf(std::ios_base::floatfield);
  file.close();
}

/**writes a spectra to the file using an iterator
@param spectraItr :: a set<int> iterator pointing to a set of workspace IDs to
be saved
@param file :: the file writer object
*/
void SaveAscii2::writeSpectra(const std::set<int>::const_iterator &spectraItr,
                              std::ofstream &file) {
  auto spec = m_ws->getSpectrum(*spectraItr);
  const auto specNo = spec->getSpectrumNo();
  const auto workspaceIndex = m_specToIndexMap[specNo];
  for (auto iter = m_metaData.begin(); iter != m_metaData.end(); ++iter) {
    auto value = m_metaDataMap[*iter][workspaceIndex];
    file << value;
    if (iter != m_metaData.end() - 1) {
      file << " " << m_sep << " ";
    }
  }
  file << std::endl;

  for (int bin = 0; bin < m_nBins; bin++) {
    if (!m_isCommonBins) // checking for ragged workspace
    {
      file << (m_ws->readX(*spectraItr)[bin] +
               m_ws->readX(*spectraItr)[bin + 1]) /
                  2;
    }

    else if (m_isHistogram & m_isCommonBins) // bin centres,
    {
      file << (m_ws->readX(0)[bin] + m_ws->readX(0)[bin + 1]) / 2;
    }

    else {
      file << m_ws->readX(0)[bin];
    }
    file << m_sep;
    file << m_ws->readY(*spectraItr)[bin];

    file << m_sep;
    file << m_ws->readE(*spectraItr)[bin];
    if (m_writeDX) {
      if (m_isHistogram) // bin centres
      {
        file << m_sep;
        file << (m_ws->readDx(0)[bin] + m_ws->readDx(0)[bin + 1]) / 2;
      } else // data points
      {
        file << m_sep;
        file << m_ws->readDx(0)[bin];
      }
    }
    file << std::endl;
  }
}

/**writes a spectra to the file using a workspace ID
@param spectraIndex :: an integer relating to a workspace ID
@param file :: the file writer object
*/
void SaveAscii2::writeSpectra(const int &spectraIndex, std::ofstream &file) {
  auto spec = m_ws->getSpectrum(spectraIndex);
  const auto specNo = spec->getSpectrumNo();
  const auto workspaceIndex = m_specToIndexMap[specNo];
  for (auto iter = m_metaData.begin(); iter != m_metaData.end(); ++iter) {
    auto value = m_metaDataMap[*iter][workspaceIndex];
    file << value;
    if (iter != m_metaData.end() - 1) {
      file << " " << m_sep << " ";
    }
  }
  file << std::endl;

  for (int bin = 0; bin < m_nBins; bin++) {
    if (m_isHistogram & m_isCommonBins) // bin centres,
    {
      file << (m_ws->readX(0)[bin] + m_ws->readX(0)[bin + 1]) / 2;
    } else if (!m_isCommonBins) // checking for ragged workspace
    {
      file << (m_ws->readX(spectraIndex)[bin] +
               m_ws->readX(spectraIndex)[bin + 1]) /
                  2;
    } else // data points
    {
      file << m_ws->readX(0)[bin];
    }
    file << m_sep;
    file << m_ws->readY(spectraIndex)[bin];

    file << m_sep;
    file << m_ws->readE(spectraIndex)[bin];
    if (m_writeDX) {
      if (m_isHistogram) // bin centres
      {
        file << m_sep;
        file << (m_ws->readDx(0)[bin] + m_ws->readDx(0)[bin + 1]) / 2;
      } else // data points
      {
        file << m_sep;
        file << m_ws->readDx(0)[bin];
      }
    }
    file << std::endl;
  }
}

/**
 * Converts a comma separated list to a vector of strings
 * Also ensures all strings are valid input
 * @param inputString	:: The user input comma separated string list
 * @return A vector of valid meta data strings
 */
std::vector<std::string>
SaveAscii2::stringListToVector(const std::string &inputString) {
  std::vector<std::string> stringVector;
  const std::vector<std::string> validMetaData{"SpectrumNumber", "Q", "Angle"};
  stringVector =
      Kernel::VectorHelper::splitStringIntoVector<std::string>(inputString);
  for (auto iter = stringVector.begin(); iter != stringVector.end(); ++iter) {
    if (std::find(validMetaData.begin(), validMetaData.end(), *iter) ==
        validMetaData.end()) {
      throw std::runtime_error(*iter + " is not recognised as a possible input "
                                       "for SpectrumMetaData.\n Valid inputs "
                                       "are: SpectrumNumber, Q, Angle.");
    }
  }
  return stringVector;
}

/**
 * Populate the map with the Q values associated with each spectrum in the
 * workspace
 */
void SaveAscii2::populateQMetaData() {
  std::vector<std::string> qValues;
  const auto nHist = m_ws->getNumberHistograms();
  for (size_t i = 0; i < nHist; i++) {
    const auto specNo = m_ws->getSpectrum(i)->getSpectrumNo();
    const auto workspaceIndex = m_specToIndexMap[specNo];
    const auto detector = m_ws->getDetector(workspaceIndex);
    double twoTheta(0.0), efixed(0.0);
    if (!detector->isMonitor()) {
      twoTheta = m_ws->detectorTwoTheta(detector) / 2.0;
      try {
        efixed = m_ws->getEFixed(detector);
      } catch (std::runtime_error error) {
        throw error;
      }
    } else {
      twoTheta = 0.0;
      efixed = DBL_MIN;
    }
    // Convert to MomentumTransfer
    auto qValue = Kernel::UnitConversion::run(twoTheta, efixed);
    auto qValueStr = boost::lexical_cast<std::string>(qValue);
    qValues.push_back(qValueStr);
  }
  m_metaDataMap["Q"] = qValues;
}

/**
 * Populate the map with the SpectrumNumber for each Spectrum in the workspace
 */
void SaveAscii2::populateSpectrumNumberMetaData() {
  std::vector<std::string> spectrumNumbers;
  const auto nHist = m_ws->getNumberHistograms();
  for (auto i = 0; i < nHist; i++) {
    const auto specNum = m_ws->getSpectrum(i)->getSpectrumNo();
    const auto specNumStr = boost::lexical_cast<std::string>(specNum);
    spectrumNumbers.push_back(specNumStr);
  }
  m_metaDataMap["SpectrumNumber"] = spectrumNumbers;
}

/**
 * Populate the map with the Angle for each spectrum in the workspace
 */
void SaveAscii2::populateAngleMetaData() {
  std::vector<std::string> angles;
  const auto nHist = m_ws->getNumberHistograms();
  for (auto i = 0; i < nHist; i++) {
    const auto specNo = m_ws->getSpectrum(i)->getSpectrumNo();
    const auto workspaceIndex = m_specToIndexMap[specNo];
    auto det = m_ws->getDetector(workspaceIndex);
    const auto two_theta = m_ws->detectorTwoTheta(det);
    const auto angle = two_theta * (180 / M_PI);
    const auto angleStr = boost::lexical_cast<std::string>(angle);
    angles.push_back(angleStr);
  }
  m_metaDataMap["Angle"] = angles;
}

/**
 * Populate all required meta data in the meta data map
 */
void SaveAscii2::populateAllMetaData() {
  for (auto iter = m_metaData.begin(); iter != m_metaData.end(); ++iter) {
    auto metaDataType = *iter;
    if (metaDataType.compare("SpectrumNumber") == 0)
      populateSpectrumNumberMetaData();
    if (metaDataType.compare("Q") == 0)
      populateQMetaData();
    if (metaDataType.compare("Angle") == 0)
      populateAngleMetaData();
  }
}

} // namespace DataHandling
} // namespace Mantid
