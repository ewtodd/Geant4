#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPad.h>
#include <TRandom3.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>
#include <iostream>
#include <vector>

// Relative FWHM calibrated to an Amptek 25 mm2 CdTe detector (1.0 to 1.5 keV
// at 122 keV), with a small quadratic term for hole trapping. Applied to the
// CZT deposits unchanged from the former plot.py.
Double_t CdTeRelativeResolution(Double_t energy_keV) {
  Double_t energy_MeV = energy_keV / 1000.0;
  Double_t constant_term = 0.008;
  Double_t statistical_term = 0.012;
  Double_t hole_trapping_term = 0.0015;
  return TMath::Sqrt(
      constant_term * constant_term +
      statistical_term * statistical_term / energy_MeV +
      hole_trapping_term * hole_trapping_term / (energy_MeV * energy_MeV));
}

Double_t BroadenEnergy(Double_t energy_keV, TRandom3 &rng) {
  if (!(energy_keV > 0))
    return energy_keV;
  Double_t fwhm_keV = CdTeRelativeResolution(energy_keV) * energy_keV;
  Double_t sigma_keV = fwhm_keV / (2 * TMath::Sqrt(2 * TMath::Log(2)));
  return rng.Gaus(energy_keV, sigma_keV);
}

Bool_t InEscapeWindow(Double_t energy_keV) {
  return energy_keV > Constants::ESCAPE_WINDOW_XMIN_KEV &&
         energy_keV < Constants::ESCAPE_WINDOW_XMAX_KEV;
}

// Every deposit of the four crystals is broadened; the window cut is applied
// to the raw and the broadened energies separately.
Bool_t ReadWindowEnergies(const TString &input_name,
                          std::vector<Double_t> &raw_keV,
                          std::vector<Double_t> &broadened_keV) {
  TString subpath = input_name + ".root";
  TString filepath = gSystem->IsAbsoluteFileName(subpath)
                         ? subpath
                         : IO::GetRootFilesBaseDir() + "/" + subpath;
  if (gSystem->AccessPathName(filepath)) {
    std::cerr << "ERROR: File " << filepath << " not found." << std::endl;
    return kFALSE;
  }
  TFile *file = IO::OpenForReading(subpath);

  // Fixed seed 113 so the broadened spectrum is the same on every run.
  TRandom3 rng(113);

  Long64_t n_total = 0;
  for (Int_t c = 0; c < Constants::N_CRYSTALS; c++) {
    TString tree_name = Form("CZT_%d", c);
    TTree *tree = static_cast<TTree *>(file->Get(tree_name));
    if (!tree) {
      std::cerr << "ERROR: Could not find tree '" << tree_name << "' in file "
                << filepath << std::endl;
      file->Close();
      delete file;
      return kFALSE;
    }

    Double_t energy_keV;
    tree->SetBranchAddress("fEDep", &energy_keV);
    Long64_t n_entries = tree->GetEntries();
    for (Long64_t i = 0; i < n_entries; i++) {
      tree->GetEntry(i);
      if (InEscapeWindow(energy_keV))
        raw_keV.push_back(energy_keV);
      Double_t smeared_keV = BroadenEnergy(energy_keV, rng);
      if (InEscapeWindow(smeared_keV))
        broadened_keV.push_back(smeared_keV);
    }
    tree->ResetBranchAddresses();

    std::cout << tree_name << ": " << n_entries << " deposits" << std::endl;
    n_total += n_entries;
  }

  std::cout << "All crystals: " << n_total << " deposits" << std::endl;
  file->Close();
  delete file;
  return kTRUE;
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

void PlotEscapeWindow(const std::vector<Double_t> &energies_keV, Int_t n_bins,
                      const TString &output_name) {
  TH1D *hist = HistogramOverDataRange(energies_keV, n_bins);
  Long64_t n_in_window = energies_keV.size();
  std::cout << output_name << ": " << n_in_window << " deposits between "
            << Constants::ESCAPE_WINDOW_XMIN_KEV << " and "
            << Constants::ESCAPE_WINDOW_XMAX_KEV << " keV, " << n_bins
            << " bins of " << 1000 * hist->GetXaxis()->GetBinWidth(1)
            << " eV from " << hist->GetXaxis()->GetXmin() << " to "
            << hist->GetXaxis()->GetXmax() << " keV" << std::endl;

  std::vector<Double_t> line_energies_keV = {
      Constants::E_CD113_NG_96KEV,
      Constants::E_CD113_NG_96KEV - Constants::E_CD_K_EDGE_KEV,
      Constants::E_CD113_NG_96KEV - Constants::E_CD_K_ALPHA_KEV,
      Constants::E_CD113_NG_75KEV};
  std::vector<TString> line_labels = {
      Form("%g keV, ^{113}Cd(n,#gamma)", Constants::E_CD113_NG_96KEV),
      Form("%g keV - Cd K-edge", Constants::E_CD113_NG_96KEV),
      Form("%g keV - Cd K_{#alpha}", Constants::E_CD113_NG_96KEV),
      Form("%g keV, ^{113}Cd(n,#gamma)", Constants::E_CD113_NG_75KEV)};
  std::vector<Int_t> line_colors = {kBlue, kGreen + 2, kRed, kGray + 1};
  Int_t n_lines = line_energies_keV.size();

  Double_t gap_px = 20;
  Double_t legend_box_px = PlottingUtils::LegendWidthPx(line_labels);
  Int_t legend_width_px = TMath::CeilNint(
      legend_box_px + 2 * gap_px - 1200 * gStyle->GetPadRightMargin());
  TPad *plot = nullptr;
  TPad *legend_pad = nullptr;
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvasWithSideLegend(
      plot, legend_pad, legend_width_px, 800, kTRUE);

  Int_t color = PlottingUtils::GetDefaultColors().at(0);
  PlottingUtils::ConfigureHistogram(hist, color,
                                    "; Energy Deposited [keV]; Counts");
  hist->SetFillStyle(1001);
  hist->SetFillColorAlpha(color, 0.5);

  // The marker lines reach 2e5 counts and the y axis always includes them.
  Double_t line_top = 2e5;
  Double_t y_min = 0.5;
  hist->SetMinimum(y_min);
  hist->SetMaximum(2 * TMath::Max(line_top, hist->GetMaximum()));
  hist->Draw("HIST");

  std::vector<TLine *> lines;
  for (Int_t i = 0; i < n_lines; i++) {
    TLine *line = new TLine(line_energies_keV[i], y_min, line_energies_keV[i],
                            line_top);
    line->SetLineColor(line_colors[i]);
    line->SetLineStyle(9);
    line->SetLineWidth(PlottingUtils::GetLineWidth() + 1);
    line->Draw();
    lines.push_back(line);
  }
  hist->Draw("AXIS SAME");

  legend_pad->cd();
  Double_t legend_pad_px = canvas->GetWw() * legend_pad->GetAbsWNDC();
  Double_t legend_top = 1 - plot->GetTopMargin();
  TLegend *legend = PlottingUtils::AddLegend(
      gap_px / legend_pad_px, (gap_px + legend_box_px) / legend_pad_px,
      legend_top - 0.065 * n_lines, legend_top);
  for (Int_t i = 0; i < n_lines; i++)
    legend->AddEntry(lines[i], line_labels[i], "l");

  PlottingUtils::SaveFigure(canvas, output_name, "", PlotSaveOptions::kLINEAR);

  delete canvas;
  for (Int_t i = 0; i < n_lines; i++)
    delete lines[i];
  delete hist;
}

// Deposits of the four CZT crystals combined, between 50 and 110 keV, as
// simulated and broadened, on a log scale with the Cd capture line, its Cd
// K-edge and K-alpha escape energies, and the 75 keV capture line marked.
void EnergyDeposition(const TString input_name = Constants::SIM_5E7) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  std::vector<Double_t> raw_keV;
  std::vector<Double_t> broadened_keV;
  if (!ReadWindowEnergies(input_name, raw_keV, broadened_keV))
    return;

  TString output_base = gSystem->BaseName(input_name);
  PlotEscapeWindow(raw_keV, Constants::ESCAPE_WINDOW_RAW_NBINS,
                   output_base + "_total_energy_deposition");
  PlotEscapeWindow(broadened_keV, Constants::ESCAPE_WINDOW_BROADENED_NBINS,
                   output_base + "_total_energy_deposition_broadened");
}
