#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include "Spectra.hpp"
#include <TFile.h>
#include <TH2D.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <iostream>
#include <vector>

void PlotLaBr3Spectra(const std::vector<Double_t> &raw_mev,
                      const std::vector<Double_t> &broadened_mev) {
  SaveSpectrum(raw_mev, Constants::SPECTRUM_NBINS, Constants::SPECTRUM_XMIN_MEV,
               Constants::SPECTRUM_XMAX_MEV, kRed,
               "Total_Energy_Deposition_in_LaBr3",
               Constants::COINCIDENCE_PLOTS);
  SaveSpectrum(raw_mev, Constants::ZOOMED_NBINS, Constants::ZOOMED_XMIN_MEV,
               Constants::ZOOMED_XMAX_MEV, kRed,
               "Total_Energy_Deposition_in_LaBr3_zoomed",
               Constants::COINCIDENCE_PLOTS);
  SaveSpectrum(broadened_mev, Constants::SPECTRUM_NBINS,
               Constants::SPECTRUM_XMIN_MEV, Constants::SPECTRUM_XMAX_MEV, kRed,
               "Total_Energy_Deposition_in_LaBr3_with_broadening",
               Constants::COINCIDENCE_PLOTS);
  SaveSpectrum(broadened_mev, Constants::ZOOMED_NBINS,
               Constants::ZOOMED_XMIN_MEV, Constants::ZOOMED_XMAX_MEV, kRed,
               "Total_Energy_Deposition_in_LaBr3_with_broadening_zoomed",
               Constants::COINCIDENCE_PLOTS);
}

void SaveCoincidenceSpectrum(const std::vector<Double_t> &energies_mev,
                             const TString &output_name) {
  SaveSpectrum(energies_mev, Constants::COINCIDENCE_SPECTRUM_NBINS,
               Constants::COINCIDENCE_SPECTRUM_XMIN_MEV,
               Constants::COINCIDENCE_SPECTRUM_XMAX_MEV, kRed, output_name,
               Constants::COINCIDENCE_PLOTS);
}

// Broadened LaBr3 against CeBr3 energy for the events whose two first-hit
// times lie within the window.
void PlotEventCoincidences(const std::vector<Double_t> &labr3_broadened_mev,
                           const std::vector<Double_t> &cebr3_broadened_mev,
                           const DetectorData &labr3,
                           const DetectorData &cebr3) {
  TH2D *coincidences = NewCoincidenceHistogram();
  FillEventCoincidences(coincidences, labr3_broadened_mev, cebr3_broadened_mev,
                        labr3, cebr3);
  SaveCoincidenceHistogram(coincidences, Constants::COINCIDENCE_PLOTS);
  delete coincidences;
}

// The former plot.py: the LaBr3 spectrum of spectrum_input, raw and broadened,
// then the LaBr3 and CeBr3 spectra, the LaBr3 against CeBr3 coincidences and
// the time distributions of coincidence_input.
void Coincidence(TString spectrum_input = Constants::SPECTRUM_INPUT,
                 TString coincidence_input = Constants::COINCIDENCE_INPUT) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");
  TRandom3 rng(Constants::BROADENING_SEED);

  TFile *spectrum_file = OpenInput(spectrum_input);
  if (!spectrum_file)
    return;
  std::vector<Double_t> spectrum_raw_mev;
  Bool_t loaded = LoadBranch(spectrum_file, Constants::LABR3_TREE,
                             Constants::EDEP_BRANCH, spectrum_raw_mev);
  spectrum_file->Close();
  delete spectrum_file;
  if (!loaded)
    return;

  std::vector<Double_t> spectrum_broadened_mev =
      BroadenEnergies(spectrum_raw_mev, LaBr3RelativeFWHM, rng);
  PlotLaBr3Spectra(spectrum_raw_mev, spectrum_broadened_mev);

  TFile *coincidence_file = OpenInput(coincidence_input);
  if (!coincidence_file)
    return;
  DetectorData labr3;
  DetectorData cebr3;
  loaded = LoadDetectorData(coincidence_file, Constants::LABR3_TREE, labr3) &&
           LoadDetectorData(coincidence_file, Constants::CEBR3_TREE, cebr3);
  coincidence_file->Close();
  delete coincidence_file;
  if (!loaded)
    return;
  if (labr3.time_ns.size() != cebr3.time_ns.size()) {
    std::cerr << "Error: the LaBr3 and CeBr3 trees of " << coincidence_input
              << " differ in length, so their rows are not events" << std::endl;
    return;
  }

  // As in the former plot.py, this is spectrum_input's raw spectrum, not
  // coincidence_input's.
  SaveCoincidenceSpectrum(spectrum_raw_mev,
                          "Total_Energy_Deposition_in_LaBr3_coincidence_data");

  std::vector<Double_t> labr3_broadened_mev =
      BroadenEnergies(labr3.energy_mev, LaBr3RelativeFWHM, rng);
  SaveCoincidenceSpectrum(
      labr3_broadened_mev,
      "Total_Energy_Deposition_in_LaBr3_with_broadening_coincidence_data");
  std::vector<Double_t> cebr3_broadened_mev =
      BroadenEnergies(cebr3.energy_mev, NaIRelativeFWHM, rng);
  SaveCoincidenceSpectrum(
      cebr3_broadened_mev,
      "Total_Energy_Deposition_in_CeBr3_with_broadening_coincidence_data");

  PlotEventCoincidences(labr3_broadened_mev, cebr3_broadened_mev, labr3, cebr3);

  SaveTimeDistribution(labr3.time_ns, "LaBr3 Time Distribution",
                       "LaBr3_Time_Distribution_coincidence_data",
                       Constants::COINCIDENCE_PLOTS);
  SaveTimeDistribution(cebr3.time_ns, "CeBr3 Time Distribution",
                       "CeBr3_Time_Distribution_coincidence_data",
                       Constants::COINCIDENCE_PLOTS);
}
