#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TColor.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TList.h>
#include <TMath.h>
#include <TObjArray.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <iomanip>
#include <vector>

struct QuenchingFactor {
  Double_t yield_percent;
  Double_t uncertainty_percent;
  // Share of the variance of E_ion = E_gamma_nr - E_gamma due to E_gamma.
  Double_t gamma_contribution;
};

QuenchingFactor CalculateQuenchingFactor(Double_t e_gamma_nr,
                                         Double_t sigma_e_gamma_nr,
                                         Double_t e_nr, Double_t sigma_e_nr,
                                         Double_t e_gamma,
                                         Double_t sigma_e_gamma) {
  QuenchingFactor result = {0, 0, 0};
  Double_t e_ion = e_gamma_nr - e_gamma;
  if (e_ion <= 0)
    return result;

  Double_t variance_e_ion =
      sigma_e_gamma_nr * sigma_e_gamma_nr + sigma_e_gamma * sigma_e_gamma;
  Double_t yield = e_ion / e_nr;
  Double_t relative_variance = variance_e_ion / (e_ion * e_ion) +
                               (sigma_e_nr * sigma_e_nr) / (e_nr * e_nr);

  result.yield_percent = yield * 100;
  result.uncertainty_percent = yield * TMath::Sqrt(relative_variance) * 100;
  result.gamma_contribution = sigma_e_gamma * sigma_e_gamma / variance_e_ion;
  return result;
}

// The i-th of n evenly spaced points from first to last, as numpy.linspace.
Double_t GridPoint(Int_t i, Int_t n, Double_t first, Double_t last) {
  return first + i * (last - first) / (n - 1);
}

// A histogram whose bin centres are the grid points, so each bin holds the
// value computed at its centre.
TH2D *CreateGridHistogram(Int_t nx, Double_t x_first, Double_t x_last, Int_t ny,
                          Double_t y_first, Double_t y_last) {
  Double_t half_dx = 0.5 * (x_last - x_first) / (nx - 1);
  Double_t half_dy = 0.5 * (y_last - y_first) / (ny - 1);
  return new TH2D(PlottingUtils::GetRandomName(), "", nx, x_first - half_dx,
                  x_last + half_dx, ny, y_first - half_dy, y_last + half_dy);
}

// Draws the contour of hist at level as a white line on canvas, labelled where
// it passes label_x, as matplotlib's clabel does. ROOT hands the contour
// lines out only through a painted "CONT LIST", so they are painted once on a
// scratch canvas and copied.
void DrawLabelledContour(TH2D *hist, TCanvas *canvas, Double_t level,
                         const TString label, Double_t label_x) {
  TH2D *contour_hist =
      static_cast<TH2D *>(hist->Clone(PlottingUtils::GetRandomName()));
  Double_t levels[1] = {level};
  contour_hist->SetContour(1, levels);

  TCanvas *scratch = new TCanvas(PlottingUtils::GetRandomName(), "",
                                 canvas->GetWw(), canvas->GetWh());
  contour_hist->Draw("CONT LIST");
  scratch->Update();

  std::vector<TGraph *> lines;
  TObjArray *contours = static_cast<TObjArray *>(
      gROOT->GetListOfSpecials()->FindObject("contours"));
  if (contours && contours->GetSize() > 0) {
    TList *level_graphs = static_cast<TList *>(contours->At(0));
    Int_t n_graphs = level_graphs->GetSize();
    for (Int_t i = 0; i < n_graphs; i++) {
      TGraph *graph = static_cast<TGraph *>(level_graphs->At(i));
      lines.push_back(static_cast<TGraph *>(graph->Clone()));
    }
  }
  delete scratch;
  delete contour_hist;

  canvas->cd();
  canvas->Update();
  Int_t n_lines = lines.size();
  if (n_lines == 0) {
    std::cerr << "No contour found at " << level << std::endl;
    return;
  }

  TGraph *longest = lines.at(0);
  for (Int_t i = 0; i < n_lines; i++) {
    TGraph *line = lines.at(i);
    line->SetLineColor(kWhite);
    line->SetLineWidth(2 * PlottingUtils::GetLineWidth());
    line->Draw("L");
    if (line->GetN() > longest->GetN())
      longest = line;
  }

  Int_t n_points = longest->GetN();
  Int_t index = 0;
  for (Int_t i = 1; i < n_points; i++) {
    if (TMath::Abs(longest->GetX()[i] - label_x) <
        TMath::Abs(longest->GetX()[index] - label_x))
      index = i;
  }
  Int_t before = TMath::Max(index - 1, 0);
  Int_t after = TMath::Min(index + 1, n_points - 1);

  // TLatex rotates in pixels, so the slope is taken in pixels too.
  Double_t pixels_per_x =
      canvas->GetWw() *
      (1 - canvas->GetLeftMargin() - canvas->GetRightMargin()) /
      (canvas->GetUxmax() - canvas->GetUxmin());
  Double_t pixels_per_y =
      canvas->GetWh() *
      (1 - canvas->GetTopMargin() - canvas->GetBottomMargin()) /
      (canvas->GetUymax() - canvas->GetUymin());
  Double_t dx_pixels =
      (longest->GetX()[after] - longest->GetX()[before]) * pixels_per_x;
  Double_t dy_pixels =
      (longest->GetY()[after] - longest->GetY()[before]) * pixels_per_y;
  Double_t angle = TMath::ATan2(dy_pixels, dx_pixels) * TMath::RadToDeg();
  if (angle > 90)
    angle -= 180;
  if (angle < -90)
    angle += 180;

  TLatex *text =
      new TLatex(longest->GetX()[index], longest->GetY()[index], label);
  text->SetTextAlign(21);
  text->SetTextAngle(angle);
  text->SetTextColor(kWhite);
  text->SetTextFont(42);
  text->SetTextSize(0.045);
  text->Draw();
}

// The common 2-D style with a linear z axis. ROOT leaves bins of zero content
// unpainted, so the frame takes the bottom colour of the palette, which is
// what the original imshow maps show there.
void DrawLinearMap(TH2D *map, TCanvas *canvas, const TString title) {
  PlottingUtils::Configure2DHistogram(map, canvas, title);
  canvas->SetLogz(kFALSE);
  canvas->SetLeftMargin(0.2);
  canvas->SetRightMargin(0.18);
  canvas->SetFrameFillColor(gStyle->GetColorPalette(0));
  map->GetXaxis()->SetLabelOffset(0.02);
  map->GetYaxis()->SetTitleOffset(1.6);
  map->Draw("COLZ");
}

void PlotIonizationYieldMap() {
  Int_t n_points = Constants::YIELD_MAP_N_POINTS;
  TH2D *yield_map =
      CreateGridHistogram(n_points, Constants::YIELD_MAP_E_GAMMA_MIN_KEV,
                          Constants::YIELD_MAP_E_GAMMA_MAX_KEV, n_points,
                          Constants::YIELD_MAP_E_GAMMA_NR_MIN_KEV,
                          Constants::YIELD_MAP_E_GAMMA_NR_MAX_KEV);

  for (Int_t i = 0; i < n_points; i++) {
    Double_t e_gamma_nr =
        GridPoint(i, n_points, Constants::YIELD_MAP_E_GAMMA_NR_MIN_KEV,
                  Constants::YIELD_MAP_E_GAMMA_NR_MAX_KEV);
    for (Int_t j = 0; j < n_points; j++) {
      Double_t e_gamma =
          GridPoint(j, n_points, Constants::YIELD_MAP_E_GAMMA_MIN_KEV,
                    Constants::YIELD_MAP_E_GAMMA_MAX_KEV);
      QuenchingFactor qf = CalculateQuenchingFactor(
          e_gamma_nr, Constants::SIGMA_E_GAMMA_NR_KEV, Constants::E_NR_KEV,
          Constants::SIGMA_E_NR_KEV, e_gamma,
          Constants::YIELD_MAP_SIGMA_E_GAMMA_KEV);
      yield_map->SetBinContent(j + 1, i + 1, qf.yield_percent);
    }
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  DrawLinearMap(yield_map, canvas,
                "; E_{#gamma} [keV]; E_{#gammanr} [keV]; Ionization Yield [%]");

  DrawLabelledContour(yield_map, canvas, Constants::YIELD_CONTOUR_PERCENT,
                      Form("%.1f%%", Constants::YIELD_CONTOUR_PERCENT), 68.715);

  std::vector<Int_t> colors;
  colors.push_back(TColor::GetColor("#ffc0cb"));
  colors.push_back(TColor::GetColor("#800080"));
  colors.push_back(TColor::GetColor("#ffff00"));

  TLegend *legend = PlottingUtils::AddLegend(0.55, 0.8, 0.2, 0.4);
  Int_t n_measurements = Constants::MEASUREMENTS.size();
  for (Int_t i = 0; i < n_measurements; i++) {
    Measurement measurement = Constants::MEASUREMENTS.at(i);
    TGraphErrors *point = new TGraphErrors(1);
    point->SetPoint(0, measurement.e_gamma_keV, measurement.e_gamma_nr_keV);
    point->SetPointError(0, measurement.e_gamma_error_keV,
                         measurement.e_gamma_nr_error_keV);
    PlottingUtils::ConfigureGraph(point, colors.at(i));
    point->Draw("P");
    legend->AddEntry(point, measurement.label, "lp");
  }

  PlottingUtils::SaveFigure(canvas, "quenching_factor_Egamma_Egammanr", "",
                            PlotSaveOptions::kLINEAR);
  delete canvas;
  delete yield_map;
}

void PlotGammaContributionMap() {
  Int_t n_points = Constants::CONTRIBUTION_MAP_N_POINTS;
  TH2D *contribution_map =
      CreateGridHistogram(n_points, Constants::CONTRIBUTION_MAP_E_GAMMA_MIN_KEV,
                          Constants::CONTRIBUTION_MAP_E_GAMMA_MAX_KEV, n_points,
                          Constants::CONTRIBUTION_MAP_SIGMA_E_GAMMA_MIN_KEV,
                          Constants::CONTRIBUTION_MAP_SIGMA_E_GAMMA_MAX_KEV);

  for (Int_t i = 0; i < n_points; i++) {
    Double_t e_gamma =
        GridPoint(i, n_points, Constants::CONTRIBUTION_MAP_E_GAMMA_MIN_KEV,
                  Constants::CONTRIBUTION_MAP_E_GAMMA_MAX_KEV);
    for (Int_t j = 0; j < n_points; j++) {
      Double_t sigma_e_gamma = GridPoint(
          j, n_points, Constants::CONTRIBUTION_MAP_SIGMA_E_GAMMA_MIN_KEV,
          Constants::CONTRIBUTION_MAP_SIGMA_E_GAMMA_MAX_KEV);
      QuenchingFactor qf = CalculateQuenchingFactor(
          Constants::CONTRIBUTION_E_GAMMA_NR_KEV,
          Constants::SIGMA_E_GAMMA_NR_KEV, Constants::E_NR_KEV,
          Constants::SIGMA_E_NR_KEV, e_gamma, sigma_e_gamma);
      contribution_map->SetBinContent(i + 1, j + 1,
                                      qf.gamma_contribution * 100);
    }
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  DrawLinearMap(
      contribution_map, canvas,
      "Contribution of E_{#gamma} to total uncertainty; "
      "E_{#gamma} [keV]; #sigma_{E_{#gamma}} [keV]; Contribution [%]");

  DrawLabelledContour(
      contribution_map, canvas, Constants::CONTRIBUTION_CONTOUR_PERCENT,
      Form("%.0f%%", Constants::CONTRIBUTION_CONTOUR_PERCENT), 68.745);

  TGraph *iaea = new TGraph(1);
  iaea->SetPoint(0, Constants::IAEA_E_GAMMA_KEV,
                 Constants::IAEA_SIGMA_E_GAMMA_KEV);
  // Black, not the original red, which vanishes into turbo's red end. Style
  // 70 is the x of style 5 drawn with a thicker line.
  iaea->SetMarkerStyle(70);
  iaea->SetMarkerSize(4);
  iaea->SetMarkerColor(kBlack);
  iaea->Draw("P");

  TLegend *legend = PlottingUtils::AddLegend(0.5, 0.8, 0.75, 0.85);
  legend->AddEntry(iaea, "IAEA Database", "p");

  PlottingUtils::SaveFigure(canvas, "contribution_plot", "",
                            PlotSaveOptions::kLINEAR);
  delete canvas;
  delete contribution_map;
}

void PrintMeasuredQuenchingFactors() {
  std::cout << "Ionization yield of each marked measurement, with its own "
               "quoted uncertainties:"
            << std::endl;
  Int_t n_measurements = Constants::MEASUREMENTS.size();
  for (Int_t i = 0; i < n_measurements; i++) {
    Measurement measurement = Constants::MEASUREMENTS.at(i);
    QuenchingFactor qf = CalculateQuenchingFactor(
        measurement.e_gamma_nr_keV, measurement.e_gamma_nr_error_keV,
        Constants::E_NR_KEV, Constants::SIGMA_E_NR_KEV, measurement.e_gamma_keV,
        measurement.e_gamma_error_keV);
    std::cout << "  " << measurement.label << ": " << std::fixed
              << std::setprecision(1) << qf.yield_percent << " +/- "
              << qf.uncertainty_percent << " %" << std::endl;
  }

  QuenchingFactor iaea = CalculateQuenchingFactor(
      Constants::CONTRIBUTION_E_GAMMA_NR_KEV, Constants::SIGMA_E_GAMMA_NR_KEV,
      Constants::E_NR_KEV, Constants::SIGMA_E_NR_KEV,
      Constants::IAEA_E_GAMMA_KEV, Constants::IAEA_SIGMA_E_GAMMA_KEV);
  std::cout << "Contribution of E_gamma to the uncertainty at the IAEA point: "
            << std::fixed << std::setprecision(1)
            << iaea.gamma_contribution * 100 << " %" << std::endl;
}

void QuenchingFactorUncertainty() {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");
  PlotIonizationYieldMap();
  PlotGammaContributionMap();
  PrintMeasuredQuenchingFactors();
}
