#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPad.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <TTree.h>
#include <iostream>
#include <vector>

struct PhotopeakCounts {
  Double_t counts;
  Double_t counts_error;
  Bool_t valid;
};

// FWHM/E of the LaBr3 crystal.
Double_t LaBr3RelativeFWHM(Double_t energy_mev) {
  Double_t relative_sigma = TMath::Sqrt(
      Constants::LABR3_RESOLUTION_A * Constants::LABR3_RESOLUTION_A +
      Constants::LABR3_RESOLUTION_B * Constants::LABR3_RESOLUTION_B /
          energy_mev +
      Constants::LABR3_RESOLUTION_C * Constants::LABR3_RESOLUTION_C /
          (energy_mev * energy_mev));
  return Constants::FWHM_PER_SIGMA * relative_sigma;
}

// Deposits of zero or less pass through unbroadened.
Double_t BroadenEnergy(Double_t energy_mev, TRandom3 &rng) {
  if (energy_mev <= 0)
    return energy_mev;
  Double_t fwhm_mev = LaBr3RelativeFWHM(energy_mev) * energy_mev;
  return rng.Gaus(energy_mev, fwhm_mev / Constants::FWHM_PER_SIGMA);
}

Bool_t InFitRange(Double_t energy_mev) {
  return energy_mev >= Constants::FIT_XMIN_MEV &&
         energy_mev <= Constants::FIT_XMAX_MEV;
}

TH1D *NewSpectrum(Int_t nbins, Double_t xmin_mev, Double_t xmax_mev) {
  return new TH1D(PlottingUtils::GetRandomName(), "", nbins, xmin_mev,
                  xmax_mev);
}

TString CountsPerBinTitle(TH1D *spectrum) {
  return Form("Counts / %.2f keV",
              spectrum->GetBinWidth(1) * Constants::MEV_TO_KEV);
}

Bool_t FillSpectra(TFile *file, TH1D *raw, TH1D *raw_zoomed, TH1D *broadened,
                   TH1D *broadened_zoomed, TH1D *peak) {
  TTree *tree = static_cast<TTree *>(file->Get(Constants::EDEP_TREE));
  if (!tree) {
    std::cerr << "Error: " << Constants::EDEP_TREE << " tree not found in "
              << file->GetName() << std::endl;
    return kFALSE;
  }
  Double_t edep_mev;
  tree->SetBranchStatus("*", 0);
  tree->SetBranchStatus(Constants::EDEP_BRANCH, 1);
  tree->SetBranchAddress(Constants::EDEP_BRANCH, &edep_mev);

  TRandom3 rng(Constants::BROADENING_SEED);
  Long64_t n_entries = tree->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    raw->Fill(edep_mev);
    raw_zoomed->Fill(edep_mev);
    Double_t broadened_mev = BroadenEnergy(edep_mev, rng);
    broadened->Fill(broadened_mev);
    broadened_zoomed->Fill(broadened_mev);
    peak->Fill(broadened_mev);
  }
  return kTRUE;
}

void SaveSpectrum(TH1D *spectrum, const TString &output_name) {
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(
      spectrum, kRed, ";Energy Deposited [MeV];" + CountsPerBinTitle(spectrum));
  spectrum->SetFillColorAlpha(kRed, 0.5);
  spectrum->SetFillStyle(1001);
  spectrum->Draw("HIST");
  PlottingUtils::SaveFigure(canvas, output_name, "", PlotSaveOptions::kLINEAR);
  delete canvas;
}

// Residuals at the centres of the bins in the fit range, over their standard
// deviation (np.std's, over n).
TGraph *StandardizedResiduals(TH1D *peak_hist, TF1 *model) {
  std::vector<Double_t> centers_mev;
  std::vector<Double_t> residuals;
  for (Int_t bin = 1; bin <= peak_hist->GetNbinsX(); bin++) {
    Double_t center_mev = peak_hist->GetBinCenter(bin);
    if (!InFitRange(center_mev))
      continue;
    centers_mev.push_back(center_mev);
    residuals.push_back(peak_hist->GetBinContent(bin) -
                        model->Eval(center_mev));
  }

  Int_t n_points = residuals.size();
  Double_t mean = 0;
  for (Int_t i = 0; i < n_points; i++)
    mean += residuals[i];
  mean /= n_points;
  Double_t variance = 0;
  for (Int_t i = 0; i < n_points; i++)
    variance += (residuals[i] - mean) * (residuals[i] - mean);
  variance /= n_points;
  Double_t standard_deviation = TMath::Sqrt(variance);

  if (standard_deviation > 0)
    for (Int_t i = 0; i < n_points; i++)
      residuals[i] /= standard_deviation;

  return new TGraph(n_points, centers_mev.data(), residuals.data());
}

void PlotPhotopeakFit(TH1D *peak_hist, TF1 *model, TF1 *photopeak,
                      TF1 *background) {
  TPad *plot_pad;
  TPad *legend_pad;
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvasWithSideLegend(
      plot_pad, legend_pad, 0, 800 + Constants::RESIDUAL_PANEL_HEIGHT_PX);
  TPad *upper;
  TPad *lower;
  PlottingUtils::SplitPadForPanel(plot_pad, Constants::RESIDUAL_PANEL_HEIGHT_PX,
                                  upper, lower);

  upper->cd();
  PlottingUtils::ConfigureHistogram(peak_hist, kBlack,
                                    ";;" + CountsPerBinTitle(peak_hist));
  peak_hist->GetXaxis()->SetLabelSize(0);
  peak_hist->GetXaxis()->SetTitleSize(0);
  peak_hist->Draw("HIST");

  Width_t line_width = PlottingUtils::GetLineWidth();
  model->SetLineColor(kRed);
  model->SetLineStyle(2);
  model->SetLineWidth(line_width);
  model->SetNpx(500);
  model->Draw("SAME");
  photopeak->SetLineColor(kGreen + 2);
  photopeak->SetLineStyle(2);
  photopeak->SetLineWidth(line_width);
  photopeak->SetNpx(500);
  photopeak->Draw("SAME");
  background->SetLineColor(kBlue);
  background->SetLineStyle(2);
  background->SetLineWidth(line_width);
  background->Draw("SAME");

  std::vector<TString> labels = {"Simulated Data", "Total Fit", "Photopeak",
                                 "Background"};
  Double_t legend_width =
      PlottingUtils::LegendWidthPx(labels) / canvas->GetWw();
  TLegend *legend =
      PlottingUtils::AddLegend(0.88 - legend_width, 0.88, 0.6, 0.86);
  legend->AddEntry(peak_hist, labels[0], "l");
  legend->AddEntry(model, labels[1], "l");
  legend->AddEntry(photopeak, labels[2], "l");
  legend->AddEntry(background, labels[3], "l");

  lower->cd();
  TH1F *frame = lower->DrawFrame(
      Constants::PEAK_HIST_XMIN_MEV, -Constants::RESIDUAL_PANEL_RANGE,
      Constants::PEAK_HIST_XMAX_MEV, Constants::RESIDUAL_PANEL_RANGE,
      ";Energy [MeV];Residuals / #sigma");
  frame->GetXaxis()->SetNdivisions(peak_hist->GetXaxis()->GetNdivisions());
  frame->GetYaxis()->SetNdivisions(503);
  frame->GetYaxis()->CenterTitle();

  TGraph *residuals = StandardizedResiduals(peak_hist, model);
  residuals->SetLineColor(kBlack);
  residuals->SetLineWidth(line_width);
  residuals->Draw("L");

  TLine *zero_line = new TLine(Constants::PEAK_HIST_XMIN_MEV, 0,
                               Constants::PEAK_HIST_XMAX_MEV, 0);
  zero_line->SetLineColor(kRed);
  zero_line->SetLineStyle(2);
  zero_line->SetLineWidth(line_width);
  zero_line->Draw();

  PlottingUtils::ScaleFigure(plot_pad);
  PlottingUtils::SaveFigure(canvas,
                            "LaBr3_fit_2MeV_peak_with_standardized_residuals",
                            "", PlotSaveOptions::kLINEAR);

  delete canvas;
  delete residuals;
  delete zero_line;
}

// Gaussian plus linear background over the bins whose centres lie in the fit
// range, and the Gaussian's integral over that range in counts.
PhotopeakCounts FitPhotopeak(TH1D *peak_hist) {
  PhotopeakCounts result = {0, 0, kFALSE};

  Double_t max_content = -TMath::Infinity();
  Double_t max_center_mev = 0;
  Double_t min_content = TMath::Infinity();
  for (Int_t bin = 1; bin <= peak_hist->GetNbinsX(); bin++) {
    Double_t center_mev = peak_hist->GetBinCenter(bin);
    if (!InFitRange(center_mev))
      continue;
    Double_t content = peak_hist->GetBinContent(bin);
    // Strictly greater keeps the first maximum, as np.argmax does.
    if (content > max_content) {
      max_content = content;
      max_center_mev = center_mev;
    }
    min_content = TMath::Min(min_content, content);
  }
  Double_t sigma_guess_mev = LaBr3RelativeFWHM(Constants::PEAK_ENERGY_MEV) *
                             Constants::PEAK_ENERGY_MEV /
                             Constants::FWHM_PER_SIGMA;

  TF1 *model =
      new TF1("photopeak_model", "[0]*exp(-0.5*((x-[1])/[2])^2) + [3]*x + [4]",
              Constants::FIT_XMIN_MEV, Constants::FIT_XMAX_MEV);
  model->SetParNames("Amplitude", "Centroid", "Sigma", "Slope", "Intercept");
  model->SetParameters(max_content - min_content, max_center_mev,
                       sigma_guess_mev, 0, min_content);
  model->SetParLimits(0, 0, TMath::Infinity());
  model->SetParLimits(1, Constants::FIT_XMIN_MEV, Constants::FIT_XMAX_MEV);
  model->SetParLimits(2, sigma_guess_mev / 10, sigma_guess_mev * 10);
  model->SetParLimits(4, 0, TMath::Infinity());

  // WW weights every bin in range, empty ones too, by one: the unweighted
  // least squares of the former plot.py's curve_fit. ROOT then scales the
  // parameter errors by chi2/ndf, as curve_fit does by default.
  TFitResultPtr fit = peak_hist->Fit(model, "RWWSQN");
  result.valid = fit.Get() && fit->IsValid();
  if (!result.valid)
    std::cerr << "Error: the 2 MeV photopeak fit failed (status "
              << (fit.Get() ? fit->Status() : -1) << ")" << std::endl;

  TF1 *photopeak = new TF1("photopeak", "[0]*exp(-0.5*((x-[1])/[2])^2)",
                           Constants::FIT_XMIN_MEV, Constants::FIT_XMAX_MEV);
  photopeak->SetParameters(model->GetParameter(0), model->GetParameter(1),
                           model->GetParameter(2));
  TF1 *background = new TF1("background", "[0]*x + [1]",
                            Constants::FIT_XMIN_MEV, Constants::FIT_XMAX_MEV);
  background->SetParameters(model->GetParameter(3), model->GetParameter(4));

  PlotPhotopeakFit(peak_hist, model, photopeak, background);

  if (result.valid) {
    const Int_t n_peak_parameters = 3;
    std::vector<Double_t> peak_covariance(n_peak_parameters *
                                          n_peak_parameters);
    for (Int_t i = 0; i < n_peak_parameters; i++)
      for (Int_t j = 0; j < n_peak_parameters; j++)
        peak_covariance[i * n_peak_parameters + j] = fit->CovMatrix(i, j);

    Double_t bin_width_mev = peak_hist->GetBinWidth(1);
    result.counts =
        photopeak->Integral(Constants::FIT_XMIN_MEV, Constants::FIT_XMAX_MEV) /
        bin_width_mev;
    result.counts_error = photopeak->IntegralError(Constants::FIT_XMIN_MEV,
                                                   Constants::FIT_XMAX_MEV,
                                                   photopeak->GetParameters(),
                                                   peak_covariance.data()) /
                          bin_width_mev;
  }

  delete model;
  delete photopeak;
  delete background;
  return result;
}

// Gammas within the tolerance of 2 MeV crossing the shell around the LiF.
Long64_t CountPeakGammas(TFile *file) {
  TTree *tree = static_cast<TTree *>(file->Get(Constants::GAMMA_TREE));
  if (!tree) {
    std::cerr << "Error: " << Constants::GAMMA_TREE << " tree not found in "
              << file->GetName() << std::endl;
    return 0;
  }
  Double_t gamma_energy_mev;
  tree->SetBranchStatus("*", 0);
  tree->SetBranchStatus(Constants::GAMMA_BRANCH, 1);
  tree->SetBranchAddress(Constants::GAMMA_BRANCH, &gamma_energy_mev);

  Long64_t n_peak_gammas = 0;
  Long64_t n_entries = tree->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    if (TMath::Abs(gamma_energy_mev - Constants::PEAK_ENERGY_MEV) <=
        Constants::GAMMA_TOLERANCE_MEV)
      n_peak_gammas++;
  }
  return n_peak_gammas;
}

// LaBr3 deposited-energy spectra, raw and broadened by the LaBr3 resolution,
// and the photopeak efficiency at 2 MeV: the fitted photopeak counts over the
// 2 MeV gammas leaving the LiF.
void PhotopeakEfficiency(TString input_name = Constants::DEFAULT_INPUT) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  TFile *file = IO::OpenForReading(input_name + ".root");
  if (file->IsZombie()) {
    std::cerr << "Error: cannot open " << input_name << ".root" << std::endl;
    delete file;
    return;
  }

  TH1D *raw =
      NewSpectrum(Constants::SPECTRUM_NBINS, Constants::SPECTRUM_XMIN_MEV,
                  Constants::SPECTRUM_XMAX_MEV);
  TH1D *raw_zoomed =
      NewSpectrum(Constants::ZOOMED_NBINS, Constants::ZOOMED_XMIN_MEV,
                  Constants::ZOOMED_XMAX_MEV);
  TH1D *broadened =
      NewSpectrum(Constants::SPECTRUM_NBINS, Constants::SPECTRUM_XMIN_MEV,
                  Constants::SPECTRUM_XMAX_MEV);
  TH1D *broadened_zoomed =
      NewSpectrum(Constants::ZOOMED_NBINS, Constants::ZOOMED_XMIN_MEV,
                  Constants::ZOOMED_XMAX_MEV);
  TH1D *peak =
      NewSpectrum(Constants::PEAK_HIST_NBINS, Constants::PEAK_HIST_XMIN_MEV,
                  Constants::PEAK_HIST_XMAX_MEV);

  if (FillSpectra(file, raw, raw_zoomed, broadened, broadened_zoomed, peak)) {
    SaveSpectrum(raw, "Total_Energy_Deposition_in_LaBr3");
    SaveSpectrum(raw_zoomed, "Total_Energy_Deposition_in_LaBr3_zoomed");
    SaveSpectrum(broadened, "Total_Energy_Deposition_in_LaBr3_with_broadening");
    SaveSpectrum(broadened_zoomed,
                 "Total_Energy_Deposition_in_LaBr3_with_broadening_zoomed");

    PhotopeakCounts photopeak = FitPhotopeak(peak);
    Long64_t n_peak_gammas = CountPeakGammas(file);

    if (photopeak.valid && n_peak_gammas > 0) {
      Double_t efficiency_percent = 100 * photopeak.counts / n_peak_gammas;
      Double_t efficiency_percent_error =
          100 * photopeak.counts_error / n_peak_gammas;
      std::cout << Form("LaBr3: Fitted counts = %.2f +/- %.2f, 2 MeV gammas "
                        "= %lld, Photopeak Efficiency (%%) = %.4f +/- %.4f",
                        photopeak.counts, photopeak.counts_error, n_peak_gammas,
                        efficiency_percent, efficiency_percent_error)
                << std::endl;
    } else {
      std::cerr << "Error: no photopeak efficiency (fit valid: "
                << photopeak.valid << ", 2 MeV gammas: " << n_peak_gammas << ")"
                << std::endl;
    }
  }

  file->Close();
  delete file;
  delete raw;
  delete raw_zoomed;
  delete broadened;
  delete broadened_zoomed;
  delete peak;
}
