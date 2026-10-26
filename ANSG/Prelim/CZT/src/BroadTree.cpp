#include "BroadTree.hh"
#include "IOUtils.hpp"
#include "PlottingUtils.hpp"
#include <TSystem.h>
#include <iostream>

BroadTree::BroadTree(const TString &filename) {
  aFilename = filename;
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
    return;
  }
  energyTreeCZT = static_cast<TTree *>(aFile->Get("BroadenedEnergyCZT"));
  energyTreeHPGe = static_cast<TTree *>(aFile->Get("BroadenedEnergyHPGe"));
  energyTreeSiLi = static_cast<TTree *>(aFile->Get("BroadenedEnergySiLi"));
  if (energyTreeCZT)
    branchEnergyDepCZT = energyTreeCZT->GetBranch("fEdepCZT");
  if (energyTreeHPGe)
    branchEnergyDepHPGe = energyTreeHPGe->GetBranch("fEdepHPGe");
  if (energyTreeSiLi)
    branchEnergyDepSiLi = energyTreeSiLi->GetBranch("fEdepSiLi");
}

BroadTree::~BroadTree() {
  std::cout << "Deleting BroadTree..." << std::endl;
  if (aFile) {
    aFile->Close();
    delete aFile;
  }
  std::cout << "Done." << std::endl;
}

TString BroadTree::getFormattedFilename() const {
  TString name = gSystem->BaseName(aFilename);
  name.ReplaceAll(".root", "");
  return name;
}

// An empty histogram if this file has no tree for the detector.
TH1D *BroadTree::energySpectrumHist(const TString detectorName,
                                    Double_t lowerBound, Double_t upperBound,
                                    Int_t nbins) {
  TBranch *branchEnergyDep = nullptr;
  if (detectorName == "CZT") {
    branchEnergyDep = branchEnergyDepCZT;
  } else if (detectorName == "HPGe") {
    branchEnergyDep = branchEnergyDepHPGe;
  } else if (detectorName == "SiLi") {
    branchEnergyDep = branchEnergyDepSiLi;
  }

  TH1D *hist = new TH1D(PlottingUtils::GetRandomName(), ";Energy (keV);Entries",
                        nbins, lowerBound, upperBound);
  hist->SetStats(0);
  if (!branchEnergyDep) {
    std::cerr << "No broadened " << detectorName << " tree in " << aFilename
              << std::endl;
    return hist;
  }

  Double_t eDep;
  branchEnergyDep->SetAddress(&eDep);
  Long64_t entries = branchEnergyDep->GetEntries();
  for (Long64_t i = 0; i < entries; i++) {
    branchEnergyDep->GetEntry(i);
    if (eDep >= lowerBound && eDep <= upperBound) {
      hist->Fill(eDep);
    }
  }
  branchEnergyDep->ResetAddress();
  return hist;
}
