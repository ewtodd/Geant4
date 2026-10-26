#ifndef TREE_MODULE_H_INCLUDED
#define TREE_MODULE_H_INCLUDED

#include <TBranch.h>
#include <TFile.h>
#include <TH1D.h>
#include <TROOT.h>
#include <TString.h>
#include <TTree.h>

class TreeModule {
private:
  TFile *aFile;
  TFile *broadenedFile;
  TTree *energyTreeCZT;
  TTree *energyTreeHPGe;
  TTree *energyTreeSiLi;
  TBranch *branchEnergyDepCZT;
  TBranch *branchEnergyDepHPGe;
  TBranch *branchEnergyDepSiLi;
  TString aFilename;

  TH1D *createHistogram(TBranch *branch, const TString &histName);
  TH1D *broadenedHist(TH1D *hist, const TString detectorName);
  void writeHistogramToTree(TTree *tree, TH1D *hist, const TString &branchName);

public:
  // The broadened trees go to broadenedFilename, by default
  // <filename without .root>_broadened.root. The input is only read.
  TreeModule(const TString &filename, const TString &broadenedFilename = "");
  ~TreeModule();
  void broadenAndStoreEnergy();
  TString getFilename() const { return aFilename; };
};

#endif // TREE_MODULE_H_INCLUDED
