#include "TreeModule.hh"
#include "IOUtils.hpp"
#include <TMath.h>
#include <iostream>

TreeModule::TreeModule(const TString &filename,
                       const TString &broadenedFilename) {
  aFilename = filename;
  broadenedFile = nullptr;
  energyTreeCZT = nullptr;
  energyTreeHPGe = nullptr;
  energyTreeSiLi = nullptr;
  branchEnergyDepCZT = nullptr;
  branchEnergyDepHPGe = nullptr;
  branchEnergyDepSiLi = nullptr;

  aFile = IO::OpenForReading(filename);
  if (aFile->IsZombie()) {
    std::cerr << "Failed to open the file: " << filename << std::endl;
    delete aFile;
    aFile = nullptr;
    // Leaves any broadened file from an earlier run untouched.
    return;
  }
  // The CZT sim writes one tree, Energy, with fEdep in keV. The HPGe and SiLi
  // trees are from an older multi-detector output and are skipped if absent.
  energyTreeCZT = static_cast<TTree *>(aFile->Get("Energy"));
  energyTreeHPGe = static_cast<TTree *>(aFile->Get("EnergyHPGe"));
  energyTreeSiLi = static_cast<TTree *>(aFile->Get("EnergySiLi"));
  if (energyTreeCZT)
    branchEnergyDepCZT = energyTreeCZT->GetBranch("fEdep");
  if (energyTreeHPGe)
    branchEnergyDepHPGe = energyTreeHPGe->GetBranch("fEdepHPGe");
  if (energyTreeSiLi)
    branchEnergyDepSiLi = energyTreeSiLi->GetBranch("fEdepSiLi");

  TString broadenedFileName = broadenedFilename;
  if (broadenedFileName == "") {
    broadenedFileName = filename;
    broadenedFileName.ReplaceAll(".root", "");
    broadenedFileName += "_broadened.root";
  }

  broadenedFile = IO::OpenForWriting(broadenedFileName, "RECREATE");
  if (broadenedFile->IsZombie()) {
    std::cerr << "Failed to open the broadened file: " << broadenedFileName
              << std::endl;
    delete broadenedFile;
    broadenedFile = nullptr;
  }
}

TreeModule::~TreeModule() {
  if (aFile) {
    aFile->Close();
    delete aFile;
  }
  if (broadenedFile) {
    broadenedFile->Close();
    delete broadenedFile;
  }
  std::cout << "Done." << std::endl;
}

TH1D *TreeModule::createHistogram(TBranch *branch, const TString &histName) {
  Double_t res = 0.;

  if (histName == "CZT") {
    res = 1.8 / 59.5;
    std::cout << "Creating CZT histogram..." << std::endl;

  } else if (histName == "HPGe") {
    std::cout << "NOT Creating HPGe histogram..." << std::endl;
    return new TH1D(histName, histName + " Spectrum;Energy (keV);Entries", 10,
                    0, 10400);

  } else if (histName == "SiLi") {
    std::cout << "NOT Creating SiLi histogram..." << std::endl;
    return new TH1D(histName, histName + " Spectrum;Energy (keV);Entries", 10,
                    0, 10400);
  }

  // Bins of a fifteenth of the FWHM at 68.752 keV, over 1.3 times the range.
  Double_t FWHM = res * 68.752;
  Int_t nbins = static_cast<Int_t>(1.3 * (10400) / (FWHM / 15));

  TH1D *hist = new TH1D(histName, histName + " Spectrum;Energy (keV);Entries",
                        nbins, 0, 10400);
  Long64_t entries = branch->GetEntries();
  std::cout << "Creating histogram " << histName << " with " << entries
            << " entries." << std::endl;

  Double_t eDep;
  branch->SetAddress(&eDep);
  for (Long64_t i = 0; i < entries; ++i) {
    branch->GetEntry(i);
    hist->Fill(eDep);
  }
  branch->ResetAddress();
  std::cout << "Filled histogram " << histName << " with " << hist->GetEntries()
            << " entries." << std::endl;
  return hist;
}

TH1D *TreeModule::broadenedHist(TH1D *hist, const TString detectorName) {
  Double_t res = 0.;

  if (detectorName == "CZT") {
    res = 1.8 / 59.5;
  } else if (detectorName == "HPGe") {
    res = 0.430 / 68.75;
  } else if (detectorName == "SiLi") {
    res = 0.165 / 5.9;
  }

  TH1D *broadSpectrum = static_cast<TH1D *>(hist->Clone());
  broadSpectrum->Reset("ICES");

  Int_t nbins = hist->GetNbinsX();
  for (Int_t i = 1; i <= nbins; ++i) {
    Double_t binContent = hist->GetBinContent(i);
    // An empty bin adds nothing, and skipping it saves most of this
    // nbins x nbins loop.
    if (binContent == 0)
      continue;
    Double_t eCenter = hist->GetBinCenter(i);
    Double_t simBinSigma = (eCenter * res) / 2.35;

    for (Int_t j = 1; j <= nbins; ++j) {
      Double_t eLow = broadSpectrum->GetBinLowEdge(j);
      Double_t eHigh = broadSpectrum->GetBinLowEdge(j + 1);

      Double_t contribution =
          binContent *
          (TMath::Erf((eHigh - eCenter) / ((TMath::Sqrt(2.0) * simBinSigma))) -
           TMath::Erf((eLow - eCenter) / ((TMath::Sqrt(2.0) * simBinSigma)))) /
          2.0;
      broadSpectrum->AddBinContent(j, contribution);
    }
  }

  broadSpectrum->SetStats(0);
  return broadSpectrum;
}

// One entry at the bin centre for each whole count in a bin; the fractional
// part of each broadened bin content is dropped.
void TreeModule::writeHistogramToTree(TTree *tree, TH1D *hist,
                                      const TString &branchName) {
  Double_t energy;
  TBranch *branch = tree->Branch(branchName, &energy, "energy/D");
  std::cout << "Writing broadened histogram to tree with branch " << branchName
            << std::endl;

  for (Int_t i = 1; i <= hist->GetNbinsX(); ++i) {
    energy = hist->GetBinCenter(i);
    Int_t entries = static_cast<Int_t>(hist->GetBinContent(i));
    for (Int_t j = 0; j < entries; ++j) {
      tree->Fill();
    }
  }
  std::cout << "Wrote " << branch->GetEntries() << " entries to branch "
            << branchName << std::endl;
  tree->ResetBranchAddresses();
}

void TreeModule::broadenAndStoreEnergy() {
  if (!broadenedFile || !broadenedFile->IsOpen()) {
    std::cerr << "Broadened file is not open or valid." << std::endl;
    return;
  }
  // The trees are created in, and written to, the broadened file.
  broadenedFile->cd();

  if (branchEnergyDepCZT) {
    TTree *broadenedTreeCZT =
        new TTree("BroadenedEnergyCZT", "Broadened Energy Spectrum for CZT");
    TH1D *histCZT = createHistogram(branchEnergyDepCZT, "CZT");
    TH1D *broadHistCZT = broadenedHist(histCZT, "CZT");
    writeHistogramToTree(broadenedTreeCZT, broadHistCZT, "fEdepCZT");
    broadenedTreeCZT->Write();
    broadenedTreeCZT->Print();
    delete histCZT;
    delete broadHistCZT;
  }

  if (branchEnergyDepHPGe) {
    TTree *broadenedTreeHPGe =
        new TTree("BroadenedEnergyHPGe", "Broadened Energy Spectrum for HPGe");
    TH1D *histHPGe = createHistogram(branchEnergyDepHPGe, "HPGe");
    TH1D *broadHistHPGe = broadenedHist(histHPGe, "HPGe");
    writeHistogramToTree(broadenedTreeHPGe, broadHistHPGe, "fEdepHPGe");
    broadenedTreeHPGe->Write();
    delete histHPGe;
    delete broadHistHPGe;
  }

  if (branchEnergyDepSiLi) {
    TTree *broadenedTreeSiLi =
        new TTree("BroadenedEnergySiLi", "Broadened Energy Spectrum for SiLi");
    TH1D *histSiLi = createHistogram(branchEnergyDepSiLi, "SiLi");
    TH1D *broadHistSiLi = broadenedHist(histSiLi, "SiLi");
    writeHistogramToTree(broadenedTreeSiLi, broadHistSiLi, "fEdepSiLi");
    broadenedTreeSiLi->Write();
    delete histSiLi;
    delete broadHistSiLi;
  }
}
