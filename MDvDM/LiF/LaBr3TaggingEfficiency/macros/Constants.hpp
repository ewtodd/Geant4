#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TMath.h>
#include <TROOT.h>
#include <TString.h>

namespace Constants {
const TString DEFAULT_INPUT = "1E9_LaBr3_Corrected";
const TString EDEP_TREE = "LaBr3";
const TString EDEP_BRANCH = "TotalEdep";
const TString GAMMA_TREE = "PreEnergy";
const TString GAMMA_BRANCH = "GammaEnergies";

const Double_t MEV_TO_KEV = 1000;

const Int_t SPECTRUM_NBINS = 3000;
const Double_t SPECTRUM_XMIN_MEV = 0.05;
const Double_t SPECTRUM_XMAX_MEV = 8;
const Int_t ZOOMED_NBINS = 750;
const Double_t ZOOMED_XMIN_MEV = 1;
const Double_t ZOOMED_XMAX_MEV = 3;

// LaBr3(Ce) 1.5"x1.5": sigma/E = sqrt(a^2 + b^2/E + c^2/E^2), E in MeV,
// matching 2.9% FWHM at 662 keV, 2.1% at 1332 keV and 1.6% at 2615 keV.
const Double_t LABR3_RESOLUTION_A = 0.00671;
const Double_t LABR3_RESOLUTION_B = 0.00895;
const Double_t LABR3_RESOLUTION_C = 0.00000213;
const Double_t FWHM_PER_SIGMA = 2 * TMath::Sqrt(2 * TMath::Log(2));

// Fixed so the broadened spectrum, and the fit to it, are reproducible.
const UInt_t BROADENING_SEED = 12345;

const Double_t PEAK_ENERGY_MEV = 2.0;
const Double_t PEAK_HIST_XMIN_MEV = 1.8;
const Double_t PEAK_HIST_XMAX_MEV = 2.4;
// The former plot.py's int(3000 * (2.4 - 1.8) / 8): 2.4 - 1.8 is just under
// 0.6 in floating point, so this truncates to 224, not 225.
const Int_t PEAK_HIST_NBINS = 224;
const Double_t FIT_XMIN_MEV = 1.95;
const Double_t FIT_XMAX_MEV = 2.12;

// np.isclose(E, 2.0, atol=0.05) also adds its default rtol of 1e-5 times 2.0.
const Double_t GAMMA_TOLERANCE_MEV = 0.05 + 1e-5 * PEAK_ENERGY_MEV;

const Int_t RESIDUAL_PANEL_HEIGHT_PX = 300;
const Double_t RESIDUAL_PANEL_RANGE = 3;
} // namespace Constants
#endif
