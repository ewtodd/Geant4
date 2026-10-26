#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TMath.h>
#include <TROOT.h>
#include <TString.h>

namespace Constants {
const TString SPECTRUM_INPUT = "leadwall1E7";
const TString COINCIDENCE_INPUT = "sumcoincidence";
const TString ANTICOINCIDENCE_INPUT = "antitest";
const TString LABR3_TREE = "LaBr3";
const TString CEBR3_TREE = "CeBr3";
const TString EDEP_BRANCH = "fEDep";
const TString TIME_BRANCH = "fTime";

// The former plot.py and newplot.py both wrote LaBr3_vs_CeBr3_Coincidence.png
// and the time distributions under the same names; separate directories keep
// both.
const TString COINCIDENCE_PLOTS = "Coincidence";
const TString ANTICOINCIDENCE_PLOTS = "AntiCoincidence";

const Double_t MEV_TO_KEV = 1000;

const Int_t SPECTRUM_NBINS = 3000;
const Double_t SPECTRUM_XMIN_MEV = 0.05;
const Double_t SPECTRUM_XMAX_MEV = 8;
const Int_t ZOOMED_NBINS = 750;
const Double_t ZOOMED_XMIN_MEV = 1;
const Double_t ZOOMED_XMAX_MEV = 3;
const Int_t COINCIDENCE_SPECTRUM_NBINS = 3000;
const Double_t COINCIDENCE_SPECTRUM_XMIN_MEV = 0.05;
const Double_t COINCIDENCE_SPECTRUM_XMAX_MEV = 3;

// The upper edge is the latest time in the data.
const Int_t TIME_NBINS = 3000;
const Double_t TIME_XMIN_NS = 0.05;

const Int_t COINCIDENCE_2D_NBINS = 100;
const Double_t COINCIDENCE_2D_XMIN_MEV = 0.05;
const Double_t COINCIDENCE_2D_XMAX_MEV = 2.5;
const Double_t COINCIDENCE_WINDOW_NS = 0.1;
// The former newplot.py paired only the earliest LaBr3 times with CeBr3 ones.
const Long64_t MAX_COINCIDENCE_POINTS = 10000;

// LaBr3(Ce) 1.5"x1.5": sigma/E = sqrt(a^2 + b^2/E + c^2/E^2), E in MeV,
// matching 2.9% FWHM at 662 keV, 2.1% at 1332 keV and 1.6% at 2615 keV.
const Double_t LABR3_RESOLUTION_A = 0.00671;
const Double_t LABR3_RESOLUTION_B = 0.00895;
const Double_t LABR3_RESOLUTION_C = 0.00000213;
// NaI(Tl) 2"x2": sqrt(a^2 + b^2/E), E in MeV, meant to match 7.0% FWHM at
// 662 keV, 5.4% at 1332 keV and 4.5% at 2615 keV.
const Double_t NAI_RESOLUTION_A = 0.026;
const Double_t NAI_RESOLUTION_B = 0.058;
const Double_t FWHM_PER_SIGMA = 2 * TMath::Sqrt(2 * TMath::Log(2));

// Fixed so the broadened spectra are reproducible.
const UInt_t BROADENING_SEED = 12345;

const TString ANSG_LABEL = "#bf{ANSG} #it{Simulation}";
// Right end of the frame, in the top margin: AddText aligns right-top.
const Double_t ANSG_LABEL_X = 0.9;
const Double_t ANSG_LABEL_Y = 0.96;

// Cf-252 Watt fission spectrum, N(E) ~ exp(-E/a) sinh(sqrt(b E))
const Double_t WATT_A_MEV = 1.025;
const Double_t WATT_B_PER_MEV = 2.926;
const Int_t WATT_NPOINTS = 250;
const Double_t WATT_LOG10_EMIN_MEV = -5;
const Double_t WATT_LOG10_EMAX_MEV = 2;
} // namespace Constants
#endif
