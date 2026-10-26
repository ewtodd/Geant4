#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>

namespace Constants {
const TString ANSG_LABEL = "#bf{ANSG} #it{Simulation}";
// Right end of the frame, in the top margin: AddText aligns right-top.
const Double_t ANSG_LABEL_X = 0.9;
const Double_t ANSG_LABEL_Y = 0.96;

const Double_t MEV_TO_EV = 1e6;

// Inputs are <MODERATOR_PREFIX><thickness>cm.root
const TString MODERATOR_PREFIX = "Poly";
const Int_t THICKNESS_MIN_CM = 5;
const Int_t THICKNESS_MAX_CM = 12;
const Int_t SPECTRUM_THICKNESS_CM = 8;
const Double_t INCIDENT_NEUTRONS = 1e6;

const TString POST_TREE = "PostEnergy";
const TString POST_ENERGY_BRANCH = "fPostEnergy";

const Double_t THERMAL_ENERGY_LOWER_MEV = 0.015e-6;
const Double_t THERMAL_ENERGY_UPPER_MEV = 0.030e-6;

const Double_t SPECTRUM_SELECTION_MIN_EV = 0.0001;
const Int_t SPECTRUM_NBINS = 250;
const Double_t SPECTRUM_XMIN_EV = 0;
const Double_t SPECTRUM_XMAX_EV = 0.15;

// Cf-252 Watt fission spectrum, N(E) ~ exp(-E/a) sinh(sqrt(b E))
const Double_t WATT_A_MEV = 1.025;
const Double_t WATT_B_PER_MEV = 2.926;
const Int_t WATT_NPOINTS = 250;
const Double_t WATT_LOG10_EMIN_MEV = -5;
const Double_t WATT_LOG10_EMAX_MEV = 2;
} // namespace Constants
#endif
