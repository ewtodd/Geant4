#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>

// Relative resolution FWHM / E = sqrt(a^2 + b^2 / E + c^2 / E^2), E in MeV.
struct Resolution {
  Double_t constant;
  Double_t statistical;
  Double_t quadratic;
};

namespace Constants {
// Every spectrum is scaled to counts per 10 keV, whatever its bin width.
const Double_t NORMALIZATION_WIDTH_KEV = 10;

// Ge and CdTe: the edges of numpy.arange(0.01, 110, 1), so the last edge is
// 109.01 keV.
const Double_t LOW_ENERGY_BIN_WIDTH_KEV = 1;
const Double_t LOW_ENERGY_XMIN_KEV = 0.01;
const Int_t LOW_ENERGY_NBINS = 109;
const Double_t LOW_ENERGY_XMAX_KEV =
    LOW_ENERGY_XMIN_KEV + LOW_ENERGY_NBINS * LOW_ENERGY_BIN_WIDTH_KEV;

// NaI: the edges of numpy.arange(5600, 6000, 5), so the last edge is 5995 keV.
const Double_t NAI_BIN_WIDTH_KEV = 5;
const Double_t NAI_XMIN_KEV = 5600;
const Int_t NAI_NBINS = 79;
const Double_t NAI_XMAX_KEV = NAI_XMIN_KEV + NAI_NBINS * NAI_BIN_WIDTH_KEV;

const Double_t COINCIDENCE_WINDOW_NS = 0.1;

// CdTe, 1 mm thick; the quadratic term stands for hole trapping.
const Resolution CDTE_RESOLUTION = {0.008, 0.012, 0.0015};
const Resolution NAI_3X3_RESOLUTION = {0.030, 0.062, 0.0};

const UInt_t BROADENING_SEED = 12345;

const TString SIM_1E8 = "1E8";
} // namespace Constants
#endif
