#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include "Spectra.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLegend.h>
#include <TMath.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <algorithm>
#include <iostream>
#include <vector>

// Row indices in order of increasing value, as np.argsort.
std::vector<Long64_t> SortedOrder(const std::vector<Double_t> &values) {
  Long64_t n_values = values.size();
  std::vector<Long64_t> order(n_values);
  for (Long64_t i = 0; i < n_values; i++)
    order[i] = i;
  std::sort(order.begin(), order.end(), [&values](Long64_t a, Long64_t b) {
    return values[a] < values[b];
  });
  return order;
}

// Where time_ns would go in sorted_times_ns, as np.searchsorted: only the
// times either side of it can be the nearest.
Long64_t InsertionIndex(Double_t time_ns,
                        const std::vector<Double_t> &sorted_times_ns) {
  return std::lower_bound(sorted_times_ns.begin(), sorted_times_ns.end(),
                          time_ns) -
         sorted_times_ns.begin();
}

Bool_t InWindow(Double_t time_ns, Long64_t index,
                const std::vector<Double_t> &sorted_times_ns) {
  Long64_t n_times = sorted_times_ns.size();
  if (index < 0 || index >= n_times)
    return kFALSE;
  return TMath::Abs(time_ns - sorted_times_ns[index]) <=
         Constants::COINCIDENCE_WINDOW_NS;
}

// The LaBr3 energies whose time has no CeBr3 time within the window. As in the
// former newplot.py, the CeBr3 times searched are those of every row in the
// file, not only the LaBr3 row's own event.
std::vector<Double_t>
AntiCoincidentEnergies(const std::vector<Double_t> &labr3_broadened_mev,
                       const std::vector<Double_t> &labr3_times_ns,
                       const std::vector<Double_t> &sorted_cebr3_times_ns) {
  std::vector<Double_t> anti_coincident_mev;
  Long64_t n_labr3 = labr3_times_ns.size();
  for (Long64_t i = 0; i < n_labr3; i++) {
    Double_t time_ns = labr3_times_ns[i];
    Long64_t next = InsertionIndex(time_ns, sorted_cebr3_times_ns);
    Bool_t coincident = InWindow(time_ns, next - 1, sorted_cebr3_times_ns) ||
                        InWindow(time_ns, next, sorted_cebr3_times_ns);
    if (!coincident)
      anti_coincident_mev.push_back(labr3_broadened_mev[i]);
  }
  return anti_coincident_mev;
}

// The former newplot.py's pairs: each of the MAX_COINCIDENCE_POINTS earliest
// LaBr3 times with whichever of its two neighbouring CeBr3 times, among all
// rows, lie within the window.
Long64_t FillCoincidencePairs(TH2D *coincidences,
                              const std::vector<Double_t> &labr3_broadened_mev,
                              const std::vector<Double_t> &cebr3_broadened_mev,
                              const DetectorData &labr3,
                              const DetectorData &cebr3) {
  std::vector<Long64_t> labr3_order = SortedOrder(labr3.time_ns);
  std::vector<Long64_t> cebr3_order = SortedOrder(cebr3.time_ns);
  Long64_t n_cebr3 = cebr3_order.size();
  std::vector<Double_t> sorted_cebr3_times_ns(n_cebr3);
  for (Long64_t j = 0; j < n_cebr3; j++)
    sorted_cebr3_times_ns[j] = cebr3.time_ns[cebr3_order[j]];

  Long64_t n_labr3 = labr3_order.size();
  Long64_t n_points = TMath::Min(n_labr3, Constants::MAX_COINCIDENCE_POINTS);
  Long64_t n_pairs = 0;
  for (Long64_t i = 0; i < n_points; i++) {
    Long64_t labr3_row = labr3_order[i];
    Double_t time_ns = labr3.time_ns[labr3_row];
    Long64_t next = InsertionIndex(time_ns, sorted_cebr3_times_ns);
    for (Long64_t j = next - 1; j <= next; j++) {
      if (!InWindow(time_ns, j, sorted_cebr3_times_ns))
        continue;
      coincidences->Fill(labr3_broadened_mev[labr3_row],
                         cebr3_broadened_mev[cebr3_order[j]]);
      n_pairs++;
    }
  }
  return n_pairs;
}

void SaveComparison(const std::vector<Double_t> &original_mev,
                    const std::vector<Double_t> &anti_coincident_mev,
                    Int_t nbins, Double_t xmin_mev, Double_t xmax_mev,
                    const TString &output_name) {
  TH1D *original = FillSpectrum(original_mev, nbins, xmin_mev, xmax_mev);
  TH1D *anti_coincident =
      FillSpectrum(anti_coincident_mev, nbins, xmin_mev, xmax_mev);

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(original, kRed,
                                    ";Energy Deposited [MeV];" +
                                        CountsPerBinTitle(original));
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

// The former newplot.py: the broadened LaBr3 spectrum before and after removing
// the events with a CeBr3 hit within the window, the time distributions, and
// the LaBr3 against CeBr3 coincidences.
void AntiCoincidence(TString input_name = Constants::ANTICOINCIDENCE_INPUT) {
  InitUtils::SetROOTPreferences();

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

  std::vector<Double_t> sorted_cebr3_times_ns(cebr3.time_ns);
  std::sort(sorted_cebr3_times_ns.begin(), sorted_cebr3_times_ns.end());
  std::vector<Double_t> anti_coincident_mev = AntiCoincidentEnergies(
      labr3_broadened_mev, labr3.time_ns, sorted_cebr3_times_ns);
  std::cout << "Found " << anti_coincident_mev.size()
            << " anti-coincidence events out of "
            << labr3_broadened_mev.size() << " total events" << std::endl;

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
                 Constants::ZOOMED_XMAX_MEV, "LaBr3_Spectrum_Comparison_zoomed");

  SaveTimeDistribution(labr3.time_ns, "LaBr3 Time Distribution",
                       "LaBr3_Time_Distribution_coincidence_data",
                       Constants::ANTICOINCIDENCE_PLOTS);
  SaveTimeDistribution(cebr3.time_ns, "CeBr3 Time Distribution",
                       "CeBr3_Time_Distribution_coincidence_data",
                       Constants::ANTICOINCIDENCE_PLOTS);

  TH2D *coincidences = NewCoincidenceHistogram();
  Long64_t n_pairs = FillCoincidencePairs(
      coincidences, labr3_broadened_mev, cebr3_broadened_mev, labr3, cebr3);
  if (n_pairs > 0)
    SaveCoincidenceHistogram(coincidences, Constants::ANTICOINCIDENCE_PLOTS);
  else
    std::cout << "No coincident events found for LaBr3_vs_CeBr3_Coincidence"
              << std::endl;
  delete coincidences;
}
