#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TMath.h>
#include <TROOT.h>
#include <TSystem.h>
#include <vector>

Double_t WattProbability(Double_t energy_mev) {
  return TMath::Exp(-energy_mev / Constants::WATT_A_MEV) *
         TMath::SinH(TMath::Sqrt(Constants::WATT_B_PER_MEV * energy_mev));
}

// The analytic Cf-252 Watt fission spectrum, normalised to its maximum.
void WattSpectrum() {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  std::vector<Double_t> energies_mev(Constants::WATT_NPOINTS);
  std::vector<Double_t> probabilities(Constants::WATT_NPOINTS);
  Double_t log10_step =
      (Constants::WATT_LOG10_EMAX_MEV - Constants::WATT_LOG10_EMIN_MEV) /
      (Constants::WATT_NPOINTS - 1);
  Double_t max_probability = 0;
  for (Int_t i = 0; i < Constants::WATT_NPOINTS; i++) {
    energies_mev[i] =
        TMath::Power(10, Constants::WATT_LOG10_EMIN_MEV + i * log10_step);
    probabilities[i] = WattProbability(energies_mev[i]);
    max_probability = TMath::Max(max_probability, probabilities[i]);
  }
  for (Int_t i = 0; i < Constants::WATT_NPOINTS; i++)
    probabilities[i] /= max_probability;

  Double_t energy_min_mev = energies_mev.front();
  Double_t energy_max_mev = energies_mev.back();

  // The curve closed down to zero at both ends, for the filled area under it.
  TGraph *area = new TGraph(Constants::WATT_NPOINTS + 2);
  for (Int_t i = 0; i < Constants::WATT_NPOINTS; i++)
    area->SetPoint(i, energies_mev[i], probabilities[i]);
  area->SetPoint(Constants::WATT_NPOINTS, energy_max_mev, 0);
  area->SetPoint(Constants::WATT_NPOINTS + 1, energy_min_mev, 0);

  TGraph *curve = new TGraph(Constants::WATT_NPOINTS, energies_mev.data(),
                             probabilities.data());

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  canvas->SetLogx(kTRUE);
  PlottingUtils::ConfigureGraph(area, kRed,
                                ";Neutron Energy [MeV];Normalized Probability");
  area->SetFillColorAlpha(kRed, 0.5);
  area->GetXaxis()->SetLimits(energy_min_mev, energy_max_mev);
  area->SetMinimum(0);
  area->SetMaximum(1.05);
  area->Draw("AF");

  PlottingUtils::ConfigureGraph(curve, kRed);
  curve->Draw("L");

  TString label = "Cf-252 Watt Spectrum";
  std::vector<TString> labels = {label};
  Double_t legend_width =
      PlottingUtils::LegendWidthPx(labels) / canvas->GetWw();
  TLegend *legend =
      PlottingUtils::AddLegend(0.18, 0.18 + legend_width, 0.78, 0.86);
  legend->AddEntry(curve, label, "l");
  PlottingUtils::AddText(Constants::ANSG_LABEL, Constants::ANSG_LABEL_X,
                         Constants::ANSG_LABEL_Y);

  PlottingUtils::SaveFigure(canvas, "prob", "", PlotSaveOptions::kLINEAR);

  delete canvas;
  delete area;
  delete curve;
}
