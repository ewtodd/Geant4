#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include "Spectra.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <iostream>
#include <vector>

// The broadened LaBr3 energies of the events in which LaBr3 was hit and CeBr3
// was not hit within the window; n_labr3_events counts the LaBr3 hits.
std::vector<Double_t>
AntiCoincidentEnergies(const std::vector<Double_t> &labr3_broadened_mev,
                       const DetectorData &labr3, const DetectorData &cebr3,
                       Long64_t &n_labr3_events) {
  std::vector<Double_t> anti_coincident_mev;
  n_labr3_events = 0;
  Long64_t n_events = labr3.time_ns.size();
  for (Long64_t i = 0; i < n_events; i++) {
    if (!HasHit(labr3.time_ns[i]))
      continue;
    n_labr3_events++;
    if (!CoincidentInEvent(labr3, cebr3, i))
      anti_coincident_mev.push_back(labr3_broadened_mev[i]);
  }
  return anti_coincident_mev;
}

void SaveComparison(const std::vector<Double_t> &original_mev,
                    const std::vector<Double_t> &anti_coincident_mev,
                    Int_t nbins, Double_t xmin_mev, Double_t xmax_mev,
                    const TString &output_name) {
  TH1D *original = FillSpectrum(original_mev, nbins, xmin_mev, xmax_mev);
  TH1D *anti_coincident =
      FillSpectrum(anti_coincident_mev, nbins, xmin_mev, xmax_mev);

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(
      original, kRed, ";Energy Deposited [MeV];" + CountsPerBinTitle(original));
  PlottingUtils::ConfigureHistogram(anti_coincident, kBlue);
  original->Draw("HIST");
  anti_coincident->Draw("HIST SAME");

  std::vector<TString> labels = {"Original LaBr3 Spectrum",
                                 "LaBr3 After Anti-Coincidence"};
  Double_t legend_width =
      PlottingUtils::LegendWidthPx(labels) / canvas->GetWw();
  TLegend *legend =
      PlottingUtils::AddLegend(0.88 - legend_width, 0.88, 0.74, 0.86);
  legend->AddEntry(original, labels[0], "l");
  legend->AddEntry(anti_coincident, labels[1], "l");

  PlottingUtils::SaveFigure(canvas, output_name,
                            Constants::ANTICOINCIDENCE_PLOTS,
                            PlotSaveOptions::kLINEAR);
  delete canvas;
  delete original;
  delete anti_coincident;
}

// The former newplot.py, matched per event: the broadened LaBr3 spectrum before
// and after removing the events with a CeBr3 hit within the window, the time
// distributions, and the LaBr3 against CeBr3 coincidences.
void AntiCoincidence(TString input_name = Constants::ANTICOINCIDENCE_INPUT) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  TFile *file = OpenInput(input_name);
  if (!file)
    return;
  DetectorData labr3;
  DetectorData cebr3;
  Bool_t loaded = LoadDetectorData(file, Constants::LABR3_TREE, labr3) &&
                  LoadDetectorData(file, Constants::CEBR3_TREE, cebr3);
  file->Close();
  delete file;
  if (!loaded)
    return;
  if (labr3.time_ns.size() != cebr3.time_ns.size()) {
    std::cerr << "Error: the LaBr3 and CeBr3 trees of " << input_name
              << " differ in length, so their rows are not events" << std::endl;
    return;
  }
  std::cout << "Loaded " << labr3.energy_mev.size() << " LaBr3 events and "
            << cebr3.energy_mev.size() << " CeBr3 events" << std::endl;

  TRandom3 rng(Constants::BROADENING_SEED);
  std::vector<Double_t> labr3_broadened_mev =
      BroadenEnergies(labr3.energy_mev, LaBr3RelativeFWHM, rng);
  std::vector<Double_t> cebr3_broadened_mev =
      BroadenEnergies(cebr3.energy_mev, LaBr3RelativeFWHM, rng);

  SaveSpectrum(labr3_broadened_mev, Constants::COINCIDENCE_SPECTRUM_NBINS,
               Constants::COINCIDENCE_SPECTRUM_XMIN_MEV,
               Constants::COINCIDENCE_SPECTRUM_XMAX_MEV, kRed,
               "Total_Energy_Deposition_in_LaBr3_with_broadening",
               Constants::ANTICOINCIDENCE_PLOTS);

  Long64_t n_labr3_events = 0;
  std::vector<Double_t> anti_coincident_mev =
      AntiCoincidentEnergies(labr3_broadened_mev, labr3, cebr3, n_labr3_events);
  std::cout << "Found " << anti_coincident_mev.size()
            << " anti-coincidence events out of " << n_labr3_events
            << " events with a LaBr3 hit" << std::endl;

  SaveSpectrum(anti_coincident_mev, Constants::COINCIDENCE_SPECTRUM_NBINS,
               Constants::COINCIDENCE_SPECTRUM_XMIN_MEV,
               Constants::COINCIDENCE_SPECTRUM_XMAX_MEV, kBlue,
               "LaBr3_Energy_Spectrum_After_Removing_Coincidence",
               Constants::ANTICOINCIDENCE_PLOTS);

  SaveComparison(labr3_broadened_mev, anti_coincident_mev,
                 Constants::COINCIDENCE_SPECTRUM_NBINS,
                 Constants::COINCIDENCE_SPECTRUM_XMIN_MEV,
                 Constants::COINCIDENCE_SPECTRUM_XMAX_MEV,
                 "LaBr3_Spectrum_Comparison");
  SaveComparison(labr3_broadened_mev, anti_coincident_mev,
                 Constants::ZOOMED_NBINS, Constants::ZOOMED_XMIN_MEV,
                 Constants::ZOOMED_XMAX_MEV,
                 "LaBr3_Spectrum_Comparison_zoomed");

  SaveTimeDistribution(labr3.time_ns, "LaBr3 Time Distribution",
                       "LaBr3_Time_Distribution_coincidence_data",
                       Constants::ANTICOINCIDENCE_PLOTS);
  SaveTimeDistribution(cebr3.time_ns, "CeBr3 Time Distribution",
                       "CeBr3_Time_Distribution_coincidence_data",
                       Constants::ANTICOINCIDENCE_PLOTS);

  TH2D *coincidences = NewCoincidenceHistogram();
  Long64_t n_coincident = FillEventCoincidences(
      coincidences, labr3_broadened_mev, cebr3_broadened_mev, labr3, cebr3);
  if (n_coincident > 0)
    SaveCoincidenceHistogram(coincidences, Constants::ANTICOINCIDENCE_PLOTS);
  else
    std::cout << "No coincident events found for LaBr3_vs_CeBr3_Coincidence"
              << std::endl;
  delete coincidences;
}
