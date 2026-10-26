#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>
#include <vector>

// A measured pair of the 68.75 keV line energies: E_gamma alone, and
// E_gamma_nr, the line plus the ionization of the nuclear recoil.
struct Measurement {
  TString label;
  Double_t e_gamma_keV;
  Double_t e_gamma_error_keV;
  Double_t e_gamma_nr_keV;
  Double_t e_gamma_nr_error_keV;
};

namespace Constants {
const Double_t SIGMA_E_GAMMA_NR_KEV = 0.0046;
const Double_t E_NR_KEV = 254.1 / 1000;
const Double_t SIGMA_E_NR_KEV = 1.5 / 1000;

// Ionization yield map over E_gamma (x) and E_gamma_nr (y)
const Int_t YIELD_MAP_N_POINTS = 200;
const Double_t YIELD_MAP_E_GAMMA_MIN_KEV = 68.650;
const Double_t YIELD_MAP_E_GAMMA_MAX_KEV = 68.8;
const Double_t YIELD_MAP_E_GAMMA_NR_MIN_KEV = 68.7;
const Double_t YIELD_MAP_E_GAMMA_NR_MAX_KEV = 68.822;
const Double_t YIELD_MAP_SIGMA_E_GAMMA_KEV = 0.007;
const Double_t YIELD_CONTOUR_PERCENT = 14.3;

const std::vector<Measurement> MEASUREMENTS = {
    {"UMich 2024", 68.752, 0.007, 68.816, 0.005},
    {"UChicago 2021", 68.734, 0.02, 68.811, 0.001},
    {"Brookhaven 1975", 68.754, 0.0043, 68.793, 0.0034}};

// Contribution map over E_gamma (x) and sigma_E_gamma (y)
const Double_t CONTRIBUTION_E_GAMMA_NR_KEV = 68.8158;
const Int_t CONTRIBUTION_MAP_N_POINTS = 100;
const Double_t CONTRIBUTION_MAP_E_GAMMA_MIN_KEV = 68.734;
const Double_t CONTRIBUTION_MAP_E_GAMMA_MAX_KEV = 68.81;
const Double_t CONTRIBUTION_MAP_SIGMA_E_GAMMA_MIN_KEV = 0.0005;
const Double_t CONTRIBUTION_MAP_SIGMA_E_GAMMA_MAX_KEV = 0.010;
const Double_t CONTRIBUTION_CONTOUR_PERCENT = 50;

const Double_t IAEA_E_GAMMA_KEV = 68.752;
const Double_t IAEA_SIGMA_E_GAMMA_KEV = 0.007;
} // namespace Constants
#endif
