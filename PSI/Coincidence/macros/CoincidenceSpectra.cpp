#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <TTree.h>

Double_t RelativeResolution(const Resolution &resolution, Double_t energy_keV) {
  Double_t energy_MeV = energy_keV / 1000.0;
  return TMath::Sqrt(
      resolution.constant * resolution.constant +
      resolution.statistical * resolution.statistical / energy_MeV +
      resolution.quadratic * resolution.quadratic / (energy_MeV * energy_MeV));
}

// Energies of zero or less are left alone, as in the original.
Double_t BroadenEnergy(Double_t energy_keV, const Resolution &resolution,
                       TRandom3 &random) {
  if (energy_keV <= 0)
    return energy_keV;
  Double_t fwhm_keV = RelativeResolution(resolution, energy_keV) * energy_keV;
  Double_t sigma_keV = fwhm_keV / (2 * TMath::Sqrt(2 * TMath::Log(2)));
  return random.Gaus(energy_keV, sigma_keV);
}

TH1D *CreateLowEnergyHistogram() {
  return new TH1D(PlottingUtils::GetRandomName(), "",
                  Constants::LOW_ENERGY_NBINS, Constants::LOW_ENERGY_XMIN_KEV,
                  Constants::LOW_ENERGY_XMAX_KEV);
}

TH1D *CreateNaIHistogram() {
  return new TH1D(PlottingUtils::GetRandomName(), "", Constants::NAI_NBINS,
                  Constants::NAI_XMIN_KEV, Constants::NAI_XMAX_KEV);
}

// Unweighted fills, so the integral before scaling is an exact count.
Long64_t CountInRange(TH1 *hist) {
  return static_cast<Long64_t>(hist->Integral());
}

void ScaleToCountsPer10keV(TH1D *hist) {
  hist->Scale(Constants::NORMALIZATION_WIDTH_KEV /
              hist->GetXaxis()->GetBinWidth(1));
}

void PlotSpectrum(TH1D *hist, Int_t color, const TString output_name) {
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureAndDrawHistogram(
      hist, color, "; Energy Deposited [keV]; Counts / 10 keV");
  PlottingUtils::SaveFigure(canvas, output_name, "", PlotSaveOptions::kLOG);
  delete canvas;
}

TTree *GetTree(TFile *file, const TString tree_name) {
  TTree *tree = static_cast<TTree *>(file->Get(tree_name));
  if (!tree)
    std::cerr << "ERROR: no tree '" << tree_name << "' in " << file->GetName()
              << std::endl;
  return tree;
}

void CoincidenceSpectra(const TString input_name = Constants::SIM_1E8) {
  TString macros_dir = gSystem->DirName(__FILE__);
  InitUtils::SetROOTPreferences(PlotSaveFormat::kPNG, macros_dir + "/plots",
                                macros_dir + "/root_files");

  TFile *file = IO::OpenForReading(input_name + ".root");
  if (file->IsZombie()) {
    std::cerr << "ERROR: cannot open " << input_name << ".root" << std::endl;
    delete file;
    return;
  }

  TTree *ge_tree = GetTree(file, "Ge");
  TTree *cdte_tree = GetTree(file, "CdTe");
  TTree *nai_tree = GetTree(file, "NaI");
  if (!ge_tree || !cdte_tree || !nai_tree) {
    file->Close();
    delete file;
    return;
  }

  // The sim adds one row to every tree per event, so entry i of each tree is
  // the same event.
  Long64_t n_events = ge_tree->GetEntries();
  if (cdte_tree->GetEntries() != n_events ||
      nai_tree->GetEntries() != n_events) {
    std::cerr << "ERROR: the Ge, CdTe and NaI trees differ in length"
              << std::endl;
    file->Close();
    delete file;
    return;
  }

  Double_t ge_edep_MeV, cdte_edep_MeV, cdte_time_ns, nai_edep_MeV, nai_time_ns;
  ge_tree->SetBranchAddress("fEDep", &ge_edep_MeV);
  cdte_tree->SetBranchAddress("fEDep", &cdte_edep_MeV);
  cdte_tree->SetBranchAddress("fTime", &cdte_time_ns);
  nai_tree->SetBranchAddress("fEDep", &nai_edep_MeV);
  nai_tree->SetBranchAddress("fTime", &nai_time_ns);

  TH1D *ge_hist = CreateLowEnergyHistogram();
  TH1D *cdte_hist = CreateLowEnergyHistogram();
  TH1D *nai_hist = CreateNaIHistogram();
  TH2D *coincidence_hist = new TH2D(
      PlottingUtils::GetRandomName(), "", Constants::LOW_ENERGY_NBINS,
      Constants::LOW_ENERGY_XMIN_KEV, Constants::LOW_ENERGY_XMAX_KEV,
      Constants::NAI_NBINS, Constants::NAI_XMIN_KEV, Constants::NAI_XMAX_KEV);

  // Fixed seed 12345 (Constants::BROADENING_SEED), so the broadened spectra
  // are reproducible.
  TRandom3 random(Constants::BROADENING_SEED);

  for (Long64_t i = 0; i < n_events; i++) {
    ge_tree->GetEntry(i);
    cdte_tree->GetEntry(i);
    nai_tree->GetEntry(i);

    Double_t cdte_keV =
        BroadenEnergy(cdte_edep_MeV * 1000, Constants::CDTE_RESOLUTION, random);
    Double_t nai_keV = BroadenEnergy(nai_edep_MeV * 1000,
                                     Constants::NAI_3X3_RESOLUTION, random);

    ge_hist->Fill(ge_edep_MeV * 1000);
    cdte_hist->Fill(cdte_keV);
    nai_hist->Fill(nai_keV);
    // A detector with no hit has time -1 ns, as the sim writes it.
    if (TMath::Abs(cdte_time_ns - nai_time_ns) <=
        Constants::COINCIDENCE_WINDOW_NS)
      coincidence_hist->Fill(cdte_keV, nai_keV);
  }
  file->Close();
  delete file;

  std::cout << "Events read: " << n_events << std::endl;
  std::cout << "Ge events in " << Constants::LOW_ENERGY_XMIN_KEV << "-"
            << Constants::LOW_ENERGY_XMAX_KEV
            << " keV: " << CountInRange(ge_hist) << std::endl;
  std::cout << "Broadened CdTe events in " << Constants::LOW_ENERGY_XMIN_KEV
            << "-" << Constants::LOW_ENERGY_XMAX_KEV
            << " keV: " << CountInRange(cdte_hist) << std::endl;
  std::cout << "Broadened NaI events in " << Constants::NAI_XMIN_KEV << "-"
            << Constants::NAI_XMAX_KEV << " keV: " << CountInRange(nai_hist)
            << std::endl;
  std::cout << "CdTe-NaI coincidences within "
            << Constants::COINCIDENCE_WINDOW_NS
            << " ns in both ranges: " << CountInRange(coincidence_hist)
            << std::endl;

  ScaleToCountsPer10keV(ge_hist);
  ScaleToCountsPer10keV(cdte_hist);
  ScaleToCountsPer10keV(nai_hist);
  Double_t pixel_area = coincidence_hist->GetXaxis()->GetBinWidth(1) *
                        coincidence_hist->GetYaxis()->GetBinWidth(1);
  coincidence_hist->Scale(Constants::NORMALIZATION_WIDTH_KEV *
                          Constants::NORMALIZATION_WIDTH_KEV / pixel_area);

  TString label = gSystem->BaseName(input_name);
  PlotSpectrum(ge_hist, kGreen + 2, label + "_Ge_energy_deposition");
  PlotSpectrum(cdte_hist, kRed + 1,
               label + "_CdTe_broadened_energy_deposition");
  PlotSpectrum(nai_hist, kBlue + 1, label + "_NaI_broadened_energy_deposition");

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureAndDraw2DHistogram(
      coincidence_hist, canvas,
      "; CdTe Deposited Energy [keV]; NaI Deposited Energy [keV]; "
      "Counts / (10 keV)^{2}");
  canvas->SetLeftMargin(0.2);
  coincidence_hist->GetYaxis()->SetTitleOffset(1.5);
  PlottingUtils::SaveFigure(canvas, label + "_CdTe_vs_NaI_coincidence", "",
                            PlotSaveOptions::kLINEAR);
  delete canvas;

  delete ge_hist;
  delete cdte_hist;
  delete nai_hist;
  delete coincidence_hist;
}
