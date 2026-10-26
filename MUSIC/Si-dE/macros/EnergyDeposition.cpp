#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPad.h>
#include <TRandom3.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>
#include <iostream>
#include <vector>

TString ConfigurationLabel(const TString &input_name) {
  Int_t n_configurations = Constants::ALL_CONFIGURATIONS.size();
  for (Int_t i = 0; i < n_configurations; i++) {
    if (input_name == Constants::ALL_CONFIGURATIONS[i])
      return Constants::CONFIGURATION_LABELS[i];
  }
  return gSystem->BaseName(input_name);
}

// Returns null, with a warning, if the file or its tree is missing.
TH1D *FillEnergyHistogram(const TString &input_name, Double_t fwhm_MeV,
                          TRandom3 &rng) {
  TString subpath = input_name + ".root";
  TString filepath = gSystem->IsAbsoluteFileName(subpath)
                         ? subpath
                         : IO::GetRootFilesBaseDir() + "/" + subpath;
  if (gSystem->AccessPathName(filepath)) {
    std::cout << "Warning: File " << filepath << " not found. Skipping."
              << std::endl;
    return nullptr;
  }

  TFile *file = IO::OpenForReading(subpath);
  TTree *tree = static_cast<TTree *>(file->Get(Constants::ENERGY_TREE));
  if (!tree) {
    std::cout << "Warning: Tree '" << Constants::ENERGY_TREE
              << "' not found in " << filepath << ". Skipping." << std::endl;
    file->Close();
    delete file;
    return nullptr;
  }

  TH1D *hist = new TH1D(PlottingUtils::GetRandomName(), "",
                        Constants::EDEP_NBINS, Constants::EDEP_XMIN_MEV,
                        Constants::EDEP_XMAX_MEV);
  Double_t sigma_MeV = fwhm_MeV / (2 * TMath::Sqrt(2 * TMath::Log(2)));

  Double_t energy_MeV;
  tree->SetBranchAddress(Constants::ENERGY_BRANCH, &energy_MeV);
  Long64_t n_entries = tree->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    if (fwhm_MeV > 0)
      hist->Fill(rng.Gaus(energy_MeV, sigma_MeV));
    else
      hist->Fill(energy_MeV);
  }

  file->Close();
  delete file;

  Long64_t n_outside = TMath::Nint(hist->GetBinContent(0) +
                                   hist->GetBinContent(Constants::EDEP_NBINS + 1));
  std::cout << input_name << ": " << n_entries << " entries, mean "
            << hist->GetMean() << " MeV, RMS " << hist->GetRMS() << " MeV, "
            << n_outside << " outside " << Constants::EDEP_XMIN_MEV << " to "
            << Constants::EDEP_XMAX_MEV << " MeV" << std::endl;
  return hist;
}

// fwhm_MeV > 0 smears every deposit with a Gaussian of that constant FWHM; the
// default 0 plots the deposits as simulated.
void EnergyDeposition(
    const std::vector<TString> input_names = Constants::ALL_CONFIGURATIONS,
    Double_t fwhm_MeV = 0) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  // Fixed seed 37 so a broadened figure is the same on every run.
  TRandom3 rng(37);

  std::vector<Int_t> colors = PlottingUtils::GetDefaultColors();
  std::vector<TH1D *> hists;
  std::vector<TString> labels;
  Double_t y_max = 0;

  Int_t n_inputs = input_names.size();
  for (Int_t i = 0; i < n_inputs; i++) {
    TH1D *hist = FillEnergyHistogram(input_names[i], fwhm_MeV, rng);
    if (!hist)
      continue;

    // Unit area inside the window, like matplotlib's density=True.
    if (hist->Integral() > 0)
      hist->Scale(1.0 / hist->Integral(), "width");

    Int_t color = colors.at(hists.size() % colors.size());
    PlottingUtils::ConfigureHistogram(hist, color);
    hist->SetFillStyle(1001);
    hist->SetFillColorAlpha(color, 0.5);

    labels.push_back(ConfigurationLabel(input_names[i]));
    hists.push_back(hist);
    y_max = TMath::Max(y_max, hist->GetMaximum());
  }

  if (hists.empty()) {
    std::cout << "No input files were read; nothing to plot." << std::endl;
    return;
  }

  Double_t gap_px = 20;
  Double_t legend_box_px = PlottingUtils::LegendWidthPx(labels);
  Int_t legend_width_px = TMath::CeilNint(
      legend_box_px + 2 * gap_px - 1200 * gStyle->GetPadRightMargin());
  TPad *plot = nullptr;
  TPad *legend_pad = nullptr;
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvasWithSideLegend(
      plot, legend_pad, legend_width_px);

  TH1D *frame = hists[0];
  frame->SetTitle("; Energy Deposition [MeV]; Probability Density [MeV^{-1}]");
  frame->SetMinimum(0);
  frame->SetMaximum(1.05 * y_max);
  frame->Draw("HIST");
  Int_t n_hists = hists.size();
  for (Int_t i = 1; i < n_hists; i++)
    hists[i]->Draw("HIST SAME");
  frame->Draw("AXIS SAME");

  TString title_first_line =
      "Simulated Energy Deposition by ^{37}Cl Ions in the Active Si";
  TString title_second_line =
      fwhm_MeV > 0
          ? Form("With Gaussian Energy Broadening (%g MeV FWHM)", fwhm_MeV)
          : "No Gaussian Energy Broadening";
  PlottingUtils::DrawTitle(plot,
                           "#splitline{" + title_first_line + "}{" +
                               title_second_line + "}",
                           0.75);

  legend_pad->cd();
  Double_t legend_pad_px = canvas->GetWw() * legend_pad->GetAbsWNDC();
  Double_t legend_top = 1 - plot->GetTopMargin();
  TLegend *legend = PlottingUtils::AddLegend(
      gap_px / legend_pad_px, (gap_px + legend_box_px) / legend_pad_px,
      legend_top - 0.065 * n_hists, legend_top);
  for (Int_t i = 0; i < n_hists; i++)
    legend->AddEntry(hists[i], labels[i], "f");

  TString output_name = "energy_deposition";
  if (fwhm_MeV > 0)
    output_name += Form("_broadened_%gMeV_fwhm", fwhm_MeV);
  PlottingUtils::SaveFigure(canvas, output_name, "", PlotSaveOptions::kLINEAR);

  delete canvas;
  for (Int_t i = 0; i < n_hists; i++)
    delete hists[i];
}
