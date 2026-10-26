#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>
#include <iostream>
#include <vector>

Bool_t ReadEnergies(const TString &subpath, std::vector<Double_t> &energies) {
  TString filepath = gSystem->IsAbsoluteFileName(subpath)
                         ? subpath
                         : IO::GetRootFilesBaseDir() + "/" + subpath;
  if (gSystem->AccessPathName(filepath)) {
    std::cout << "Warning: File " << filepath << " not found. Skipping."
              << std::endl;
    return kFALSE;
  }

  TFile *file = IO::OpenForReading(subpath);
  TTree *tree = static_cast<TTree *>(file->Get(Constants::ENERGY_TREE));
  if (!tree) {
    std::cout << "Warning: Tree '" << Constants::ENERGY_TREE
              << "' not found in " << filepath << ". Skipping." << std::endl;
    file->Close();
    delete file;
    return kFALSE;
  }

  Double_t energy_keV;
  tree->SetBranchAddress(Constants::ENERGY_BRANCH, &energy_keV);
  Long64_t n_entries = tree->GetEntries();
  energies.reserve(n_entries);
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    energies.push_back(energy_keV);
  }

  file->Close();
  delete file;
  return kTRUE;
}

void ApplyGaussianBroadening(std::vector<Double_t> &energies, TRandom3 &rng) {
  Double_t fwhm_per_keV =
      Constants::RESOLUTION_FWHM_KEV / Constants::RESOLUTION_REFERENCE_KEV;
  Int_t n_energies = energies.size();
  for (Int_t i = 0; i < n_energies; i++) {
    Double_t sigma_keV = fwhm_per_keV * energies[i] / Constants::FWHM_TO_SIGMA;
    energies[i] = rng.Gaus(energies[i], sigma_keV);
  }
}

// numpy's histogram with an integer bin count: equal bins from the smallest to
// the largest value, the last bin closed on the right so the largest value is
// counted, and (0, 1) for an empty sample.
TH1D *HistogramOverDataRange(const std::vector<Double_t> &values,
                             Int_t n_bins) {
  Int_t n_values = values.size();
  Double_t low = 0, high = 1;
  if (n_values > 0) {
    low = TMath::MinElement(n_values, values.data());
    high = TMath::MaxElement(n_values, values.data());
  }
  if (low == high) {
    low -= 0.5;
    high += 0.5;
  }

  TH1D *hist = new TH1D(PlottingUtils::GetRandomName(), "", n_bins, low, high);
  for (Int_t i = 0; i < n_values; i++)
    hist->Fill(values[i]);

  hist->SetBinContent(n_bins, hist->GetBinContent(n_bins) +
                                  hist->GetBinContent(n_bins + 1));
  hist->SetBinContent(n_bins + 1, 0);
  return hist;
}

void EnergyDeposition(
    Bool_t apply_broadening = kTRUE,
    const std::vector<TString> distances_cm = Constants::DISTANCES_CM,
    const std::vector<Int_t> angles_deg = Constants::ANGLES_DEG,
    const TString input_dir = "") {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  // Fixed seed 5486 so the broadened figure is the same on every run.
  TRandom3 rng(5486);

  std::vector<Int_t> colors = PlottingUtils::GetDefaultColors();
  std::vector<TH1D *> hists;
  std::vector<TString> labels;
  Double_t y_max = 0;

  Int_t n_distances = distances_cm.size();
  Int_t n_angles = angles_deg.size();
  for (Int_t d = 0; d < n_distances; d++) {
    for (Int_t a = 0; a < n_angles; a++) {
      TString name = Form("air_%scm_%ddeg.root", distances_cm[d].Data(),
                          angles_deg[a]);
      TString subpath = input_dir.IsNull() ? name : input_dir + "/" + name;

      std::vector<Double_t> energies;
      if (!ReadEnergies(subpath, energies))
        continue;
      if (apply_broadening)
        ApplyGaussianBroadening(energies, rng);

      TH1D *hist = HistogramOverDataRange(energies, Constants::EDEP_NBINS);
      Long64_t n_entries = energies.size();
      std::cout << name << ": " << n_entries << " entries, "
                << Constants::EDEP_NBINS << " bins of "
                << hist->GetXaxis()->GetBinWidth(1) << " keV from "
                << hist->GetXaxis()->GetXmin() << " to "
                << hist->GetXaxis()->GetXmax() << " keV" << std::endl;

      // Unit area, like matplotlib's density=True.
      if (hist->Integral() > 0)
        hist->Scale(1.0 / hist->Integral(), "width");

      Int_t color = colors.at(hists.size() % colors.size());
      PlottingUtils::ConfigureHistogram(hist, color);
      hist->SetFillStyle(1001);
      hist->SetFillColorAlpha(color, 0.5);

      TString distance_label = distances_cm[d];
      distance_label.ReplaceAll("_", ".");
      labels.push_back(
          Form("%s cm, %d#circ", distance_label.Data(), angles_deg[a]));
      hists.push_back(hist);
      y_max = TMath::Max(y_max, hist->GetMaximum());
    }
  }

  if (hists.empty()) {
    std::cout << "No input files were read; nothing to plot." << std::endl;
    return;
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  // Room for the two-line title above the y axis' x10^-3 exponent.
  canvas->SetTopMargin(0.17);

  TH1D *frame = new TH1D(PlottingUtils::GetRandomName(), "", 1,
                         Constants::EDEP_PLOT_XMIN_KEV,
                         Constants::EDEP_PLOT_XMAX_KEV);
  PlottingUtils::ConfigureHistogram(
      frame, kBlack, "; Energy Deposition [keV]; Simulated Distribution");
  frame->GetYaxis()->SetMaxDigits(2);
  frame->SetMinimum(0);
  frame->SetMaximum(1.05 * y_max);
  frame->Draw("AXIS");
  frame->Draw("AXIG SAME");

  Int_t n_hists = hists.size();
  for (Int_t i = 0; i < n_hists; i++)
    hists[i]->Draw("HIST SAME");
  frame->Draw("AXIS SAME");

  Double_t legend_right = 1 - canvas->GetRightMargin() - 0.02;
  Double_t legend_top = 1 - canvas->GetTopMargin() - 0.02;
  Double_t legend_width = PlottingUtils::LegendWidthPx(labels) / 1200.0;
  Double_t legend_height = 0.065 * n_hists;
  TLegend *legend =
      PlottingUtils::AddLegend(legend_right - legend_width, legend_right,
                               legend_top - legend_height, legend_top);
  for (Int_t i = 0; i < n_hists; i++)
    legend->AddEntry(hists[i], labels[i], "f");

  TString title_first_line = Form("Simulated Energy Deposition by %.0f keV "
                                  "Alphas in Silicon Diode in Air,",
                                  Constants::ALPHA_ENERGY_KEV);
  TString title_second_line =
      apply_broadening
          ? Form("With Gaussian Energy Broadening (%.0f keV FWHM @ %.0f keV)",
                 Constants::RESOLUTION_FWHM_KEV,
                 Constants::RESOLUTION_REFERENCE_KEV)
          : TString("No Gaussian Energy Broadening");
  PlottingUtils::DrawTitle(canvas,
                           "#splitline{" + title_first_line + "}{" +
                               title_second_line + "}",
                           0.75);

  TString output_name = apply_broadening
                            ? "energy_deposition_with_broadening"
                            : "energy_deposition_no_broadening";
  PlottingUtils::SaveFigure(canvas, output_name, "", PlotSaveOptions::kLINEAR);

  delete canvas;
  delete frame;
  for (Int_t i = 0; i < n_hists; i++)
    delete hists[i];
}
