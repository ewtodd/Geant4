#ifndef SPECTRA_HPP
#define SPECTRA_HPP
#include "Constants.hpp"
#include "IOUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TTree.h>
#include <algorithm>
#include <iostream>
#include <vector>

// Shared by Coincidence.cpp and AntiCoincidence.cpp.

// One row per event in which either detector saw a deposit; the LaBr3 and
// CeBr3 trees are filled together, so row i of each is the same event.
struct DetectorData {
  std::vector<Double_t> energy_mev;
  std::vector<Double_t> time_ns;
};

// FWHM/E of the LaBr3 crystal.
inline Double_t LaBr3RelativeFWHM(Double_t energy_mev) {
  Double_t relative_sigma = TMath::Sqrt(
      Constants::LABR3_RESOLUTION_A * Constants::LABR3_RESOLUTION_A +
      Constants::LABR3_RESOLUTION_B * Constants::LABR3_RESOLUTION_B /
          energy_mev +
      Constants::LABR3_RESOLUTION_C * Constants::LABR3_RESOLUTION_C /
          (energy_mev * energy_mev));
  return Constants::FWHM_PER_SIGMA * relative_sigma;
}

// Not converted from sigma like the LaBr3 one, but the former plot.py broadened
// with it as FWHM/E all the same, and so does this.
inline Double_t NaIRelativeFWHM(Double_t energy_mev) {
  return TMath::Sqrt(Constants::NAI_RESOLUTION_A * Constants::NAI_RESOLUTION_A +
                     Constants::NAI_RESOLUTION_B * Constants::NAI_RESOLUTION_B /
                         energy_mev);
}

// Deposits of zero or less pass through unbroadened.
inline std::vector<Double_t>
BroadenEnergies(const std::vector<Double_t> &energies_mev,
                Double_t (*relative_fwhm)(Double_t), TRandom3 &rng) {
  std::vector<Double_t> broadened_mev(energies_mev);
  Long64_t n_energies = energies_mev.size();
  for (Long64_t i = 0; i < n_energies; i++) {
    if (energies_mev[i] <= 0)
      continue;
    Double_t fwhm_mev = relative_fwhm(energies_mev[i]) * energies_mev[i];
    broadened_mev[i] =
        rng.Gaus(energies_mev[i], fwhm_mev / Constants::FWHM_PER_SIGMA);
  }
  return broadened_mev;
}

// The caller owns the file and must close and delete it; null if unusable.
inline TFile *OpenInput(const TString &input_name) {
  TFile *file = IO::OpenForReading(input_name + ".root");
  if (file->IsZombie()) {
    std::cerr << "Error: cannot open " << input_name << ".root" << std::endl;
    delete file;
    return nullptr;
  }
  return file;
}

inline Bool_t LoadBranch(TFile *file, const TString &tree_name,
                         const TString &branch_name,
                         std::vector<Double_t> &values) {
  TTree *tree = static_cast<TTree *>(file->Get(tree_name));
  if (!tree || !tree->GetBranch(branch_name)) {
    std::cerr << "Error: no " << tree_name << "/" << branch_name << " in "
              << file->GetName() << std::endl;
    return kFALSE;
  }
  Double_t value;
  tree->SetBranchStatus("*", 0);
  tree->SetBranchStatus(branch_name, 1);
  tree->SetBranchAddress(branch_name, &value);
  Long64_t n_entries = tree->GetEntries();
  values.resize(n_entries);
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    values[i] = value;
  }
  tree->ResetBranchAddresses();
  return kTRUE;
}

inline Bool_t LoadDetectorData(TFile *file, const TString &tree_name,
                               DetectorData &data) {
  return LoadBranch(file, tree_name, Constants::EDEP_BRANCH, data.energy_mev) &&
         LoadBranch(file, tree_name, Constants::TIME_BRANCH, data.time_ns);
}

inline TH1D *FillSpectrum(const std::vector<Double_t> &energies_mev,
                          Int_t nbins, Double_t xmin_mev, Double_t xmax_mev) {
  TH1D *spectrum = new TH1D(PlottingUtils::GetRandomName(), "", nbins,
                            xmin_mev, xmax_mev);
  Long64_t n_energies = energies_mev.size();
  for (Long64_t i = 0; i < n_energies; i++)
    spectrum->Fill(energies_mev[i]);
  return spectrum;
}

inline TString CountsPerBinTitle(TH1D *spectrum) {
  return Form("Counts / %.2f keV",
              spectrum->GetBinWidth(1) * Constants::MEV_TO_KEV);
}

// Filled and outlined in one colour.
inline void SaveSpectrum(const std::vector<Double_t> &energies_mev,
                         Int_t nbins, Double_t xmin_mev, Double_t xmax_mev,
                         Int_t color, const TString &output_name,
                         const TString &output_subdirectory) {
  TH1D *spectrum = FillSpectrum(energies_mev, nbins, xmin_mev, xmax_mev);
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(spectrum, color,
                                    ";Energy Deposited [MeV];" +
                                        CountsPerBinTitle(spectrum));
  spectrum->SetFillColorAlpha(color, 0.5);
  spectrum->SetFillStyle(1001);
  spectrum->Draw("HIST");
  PlottingUtils::SaveFigure(canvas, output_name, output_subdirectory,
                            PlotSaveOptions::kLINEAR);
  delete canvas;
  delete spectrum;
}

// From 0.05 ns to the latest time, on a log scale.
inline void SaveTimeDistribution(const std::vector<Double_t> &times_ns,
                                 const TString &title,
                                 const TString &output_name,
                                 const TString &output_subdirectory) {
  if (times_ns.empty())
    return;
  Double_t latest_ns = *std::max_element(times_ns.begin(), times_ns.end());
  if (latest_ns <= Constants::TIME_XMIN_NS) {
    std::cerr << "Error: no " << title << " times above "
              << Constants::TIME_XMIN_NS << " ns" << std::endl;
    return;
  }

  TH1D *distribution =
      new TH1D(PlottingUtils::GetRandomName(), "", Constants::TIME_NBINS,
               Constants::TIME_XMIN_NS, latest_ns);
  Long64_t n_times = times_ns.size();
  for (Long64_t i = 0; i < n_times; i++) {
    // numpy closes its last bin, so the latest time lands in it, not in the
    // overflow.
    if (times_ns[i] == latest_ns)
      distribution->Fill(distribution->GetBinCenter(Constants::TIME_NBINS));
    else
      distribution->Fill(times_ns[i]);
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(distribution, kBlue,
                                    title + ";Time [ns];Counts");
  // Delayed decays stretch the axis to ~1e15 ns, too long to label in full.
  distribution->GetXaxis()->SetNoExponent(kFALSE);
  distribution->SetFillColorAlpha(kBlue, 0.5);
  distribution->SetFillStyle(1001);
  distribution->Draw("HIST");
  PlottingUtils::SaveFigure(canvas, output_name, output_subdirectory,
                            PlotSaveOptions::kLOG);
  delete canvas;
  delete distribution;
}

inline TH2D *NewCoincidenceHistogram() {
  return new TH2D(PlottingUtils::GetRandomName(), "",
                  Constants::COINCIDENCE_2D_NBINS,
                  Constants::COINCIDENCE_2D_XMIN_MEV,
                  Constants::COINCIDENCE_2D_XMAX_MEV,
                  Constants::COINCIDENCE_2D_NBINS,
                  Constants::COINCIDENCE_2D_XMIN_MEV,
                  Constants::COINCIDENCE_2D_XMAX_MEV);
}

inline void SaveCoincidenceHistogram(TH2D *coincidences,
                                     const TString &output_subdirectory) {
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureAndDraw2DHistogram(
      coincidences, canvas, ";LaBr3 Energy [MeV];CeBr3 Energy [MeV];Counts");
  PlottingUtils::SaveFigure(canvas, "LaBr3_vs_CeBr3_Coincidence",
                            output_subdirectory, PlotSaveOptions::kLINEAR);
  delete canvas;
}
#endif
