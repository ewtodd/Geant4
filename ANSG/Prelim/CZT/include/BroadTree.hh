#ifndef BROADTREE_MODULE_H_INCLUDED
#define BROADTREE_MODULE_H_INCLUDED
#include <TBranch.h>
#include <TFile.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TString.h>
#include <TTree.h>

class BroadTree {

private:
  TFile *aFile;
  TTree *energyTreeCZT;
  TTree *energyTreeHPGe;
  TTree *energyTreeSiLi;
  TBranch *branchEnergyDepCZT;
  TBranch *branchEnergyDepHPGe;
  TBranch *branchEnergyDepSiLi;
  TString aFilename;

public:
  BroadTree(const TString &filename);
  ~BroadTree();
  TFile *getAFile() const { return aFile; };
  TTree *getEnergyTreeCZT() const { return energyTreeCZT; };
  TTree *getEnergyTreeHPGe() const { return energyTreeHPGe; };
  TTree *getEnergyTreeSiLi() const { return energyTreeSiLi; };
  TBranch *getBranchEnergyCZT() const { return branchEnergyDepCZT; };
  TBranch *getBranchEnergyHPGe() const { return branchEnergyDepHPGe; };
  TBranch *getBranchEnergySiLi() const { return branchEnergyDepSiLi; };
  TString getFilename() const { return aFilename; };
  TString getFormattedFilename() const;
  TH1D *energySpectrumHist(const TString detectorName, Double_t lowerBound = 0,
                           Double_t upperBound = 100, Int_t nbins = 829);
};

#endif
