#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>

namespace Constants {
// Inputs are root_files/<material><thickness>cm.root, one sim per moderator.
const TString WATER = "Water";
const TString POLY = "Poly";
const Int_t THICKNESS_MIN_CM = 10;
const Int_t THICKNESS_MAX_CM = 17;

// fPostEnergy is in MeV.
const Double_t THERMAL_ENERGY_LOWER_MEV = 0.015e-6;
const Double_t THERMAL_ENERGY_UPPER_MEV = 0.030e-6;
// The thermal count times this is plotted as the estimated flux in n/s.
const Double_t THERMAL_COUNT_SCALE = 100;

const Int_t SPECTRUM_POLY_THICKNESS_CM = 11;
const Int_t SPECTRUM_WATER_THICKNESS_CM = 15;
const Double_t SPECTRUM_LOWER_MEV = 0.001e-6;
const Double_t SPECTRUM_UPPER_MEV = 0.15e-6;
const Int_t SPECTRUM_NBINS = 50;
const Double_t SPECTRUM_XMIN_EV = 0;
const Double_t SPECTRUM_XMAX_EV = 0.15;

// GeometryOptimizationHPGe's generator samples its neutron energies from
// this histogram, read by name from thermalEnergySpectra.root.
const TString THERMAL_SPECTRA_OUTPUT = "thermalEnergySpectra";
const TString WATER_SPECTRUM_HIST_NAME = "ThermalEnergyHistWater";
} // namespace Constants
#endif
