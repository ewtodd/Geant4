#include "Constants.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TFile.h>
#include <TH1F.h>
#include <TParameter.h>
#include <TROOT.h>
#include <TTree.h>

void AddHistogram(TString filename) {
  TString filepath = "root_files/" + filename + ".root";
  TFile *file = new TFile(filepath, "UPDATE");

  TString perSecond = "";
  EColor colors[4] = {kP10Blue, kP10Red, kP10Cyan, kP10Orange};

  // Per-crystal full-range histograms
  TH1F *crystalHist[4];
  for (Int_t c = 0; c < 4; c++) {
    TString treeName = Form("CZT_%d", c);
    TTree *tree = static_cast<TTree *>(file->Get(treeName));
    if (!tree) {
      std::cerr << "ERROR: Could not find tree '" << treeName << "' in file "
                << filepath << std::endl;
      continue;
    }

    Double_t energy;
    tree->SetBranchAddress("fEDep", &energy);

    crystalHist[c] = new TH1F(
        PlottingUtils::GetRandomName(),
        Form("%s CZT_%d; Energy [keV]; Counts / %d eV%s", filename.Data(), c,
             Constants::BIN_WIDTH_EV, perSecond.Data()),
        Constants::HIST_NBINS, Constants::HIST_XMIN, Constants::HIST_XMAX);

    Int_t n_entries = tree->GetEntries();
    tree->LoadBaskets();
    for (Int_t i = 0; i < n_entries; i++) {
      tree->GetEntry(i);
      crystalHist[c]->Fill(energy);
    }

    PlottingUtils::ConfigureHistogram(crystalHist[c], colors[c]);
    crystalHist[c]->GetYaxis()->SetTitleOffset(1.2);
    crystalHist[c]->Write(Form("hist_CZT_%d", c), TObject::kOverwrite);
  }

  // Combined histogram: sum all per-crystal histograms
  TH1F *combinedHist = new TH1F(
      PlottingUtils::GetRandomName(),
      Form("%s Combined; Energy [keV]; Counts / %d eV%s", filename.Data(),
           Constants::BIN_WIDTH_EV, perSecond.Data()),
      Constants::HIST_NBINS, Constants::HIST_XMIN, Constants::HIST_XMAX);

  TH1F *zoomedHist = new TH1F(
      PlottingUtils::GetRandomName(),
      Form("%s Combined; Energy [keV]; Counts / %d eV%s", filename.Data(),
           Constants::BIN_WIDTH_EV, perSecond.Data()),
      Constants::ZOOMED_NBINS, Constants::ZOOMED_XMIN, Constants::ZOOMED_XMAX);

  TH1F *peakHist = new TH1F(
      PlottingUtils::GetRandomName(),
      Form("%s Combined; Energy [keV]; Counts / %d eV%s", filename.Data(),
           Constants::BIN_WIDTH_EV, perSecond.Data()),
      Constants::PEAK_NBINS, Constants::PEAK_XMIN, Constants::PEAK_XMAX);

  for (Int_t c = 0; c < 4; c++) {
    if (crystalHist[c])
      combinedHist->Add(crystalHist[c]);
  }

  // Fill zoomed/peak from combined bin contents
  for (Int_t bin = 1; bin <= combinedHist->GetNbinsX(); bin++) {
    Double_t center = combinedHist->GetBinCenter(bin);
    Double_t content = combinedHist->GetBinContent(bin);
    if (Constants::ZOOMED_XMIN < center && center < Constants::ZOOMED_XMAX)
      zoomedHist->SetBinContent(zoomedHist->FindBin(center), content);
    if (Constants::PEAK_XMIN < center && center < Constants::PEAK_XMAX)
      peakHist->SetBinContent(peakHist->FindBin(center), content);
  }

  PlottingUtils::ConfigureHistogram(combinedHist, kP10Violet);
  PlottingUtils::ConfigureHistogram(zoomedHist, kP10Violet);
  PlottingUtils::ConfigureHistogram(peakHist, kP10Violet);

  combinedHist->GetYaxis()->SetTitleOffset(1.2);
  zoomedHist->GetYaxis()->SetTitleOffset(1.2);
  peakHist->GetYaxis()->SetTitleOffset(1.2);

  combinedHist->Write("hist", TObject::kOverwrite);
  zoomedHist->Write("zoomedHist", TObject::kOverwrite);
  peakHist->Write("peakHist", TObject::kOverwrite);

  std::cout << "Wrote per-crystal and combined histograms for " << filename
            << std::endl;

  file->Close();
}

void AddAllHistograms(std::vector<TString> filenames) {
  Int_t n_files = filenames.size();
  for (Int_t i = 0; i < n_files; i++) {
    TString filename = filenames.at(i);
    AddHistogram(filename);
  }
}

void SimHistogram() {
  InitUtils::SetROOTPreferences();
  std::vector<TString> filenames;

  filenames.push_back(Constants::SIM_5E7);

  AddAllHistograms(filenames);
}
