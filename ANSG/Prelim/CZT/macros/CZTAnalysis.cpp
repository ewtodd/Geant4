#include "Analysis.hh"
#include "Constants.hpp"
#include "InitUtils.hpp"
#include <TROOT.h>
#include <vector>

// Built from ../src by the Makefile in the project root; the CZT dev shell
// puts ../lib on LD_LIBRARY_PATH.
R__LOAD_LIBRARY(libCZTAnalysis)

// Writes <name>_broadened.root next to each input, or into
// broadened_directory if one is given, unless broaden is kFALSE, in which
// case the broadened files from an earlier run are read.
void CZTAnalysis(
    const std::vector<TString> input_names = Constants::SIM_NAMES,
    const std::vector<TString> measurement_times = Constants::MEASUREMENT_TIMES,
    const TString broadened_directory = "", Bool_t broaden = kTRUE) {
  InitUtils::SetROOTPreferences();

  Analysis *analysis =
      new Analysis(input_names, measurement_times, broadened_directory);
  analysis->loadFiles(broaden);
  analysis->drawHists("CZT", kFALSE);
  analysis->drawPartialHists("CZT", Constants::PEAK_WINDOW_LOW_KEV,
                             Constants::PEAK_WINDOW_HIGH_KEV, kFALSE,
                             Constants::PEAK_WINDOW_NBINS);
  delete analysis;
}
