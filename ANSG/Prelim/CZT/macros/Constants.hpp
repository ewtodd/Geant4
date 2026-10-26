#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>
#include <vector>

namespace Constants {
// Runs named by their number of primaries, each labelled with the equivalent
// measurement time. The original also read a 1E9 run, which is not on
// /labdata.
const std::vector<TString> SIM_NAMES = {"1E6", "5E6", "1E7", "5E7", "1E8"};
const std::vector<TString> MEASUREMENT_TIMES = {
    "0.267 min", "1.3 min", "2.67 min", "13.3 min", "26.7 min"};

// Window around the 68.75 keV peak for the fits
const Double_t PEAK_WINDOW_LOW_KEV = 64.75;
const Double_t PEAK_WINDOW_HIGH_KEV = 72.75;
const Int_t PEAK_WINDOW_NBINS = 66;
} // namespace Constants
#endif
