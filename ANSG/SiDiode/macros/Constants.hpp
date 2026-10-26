#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>
#include <vector>

namespace Constants {

const TString ENERGY_TREE = "Energy";
const TString ENERGY_BRANCH = "fEdep";

const Double_t ALPHA_ENERGY_KEV = 5486;

// Detector resolution: 16 keV FWHM at the alpha energy, with the FWHM
// proportional to the deposited energy.
const Double_t RESOLUTION_FWHM_KEV = 16;
const Double_t RESOLUTION_REFERENCE_KEV = ALPHA_ENERGY_KEV;
// Kept from the original plotting.py rather than 2 sqrt(2 ln 2) = 2.3548.
const Double_t FWHM_TO_SIGMA = 2.35;

// Each spectrum gets this many bins between its own smallest and largest
// energy, as numpy's bins=650 does; the plot then shows a fixed window.
const Int_t EDEP_NBINS = 650;
const Double_t EDEP_PLOT_XMIN_KEV = 2250;
const Double_t EDEP_PLOT_XMAX_KEV = 3500;

// Inputs are root_files/air_<distance>cm_<angle>deg.root.
const std::vector<TString> DISTANCES_CM = {"2_5"};
const std::vector<Int_t> ANGLES_DEG = {0, 30, 60};

} // namespace Constants
#endif
