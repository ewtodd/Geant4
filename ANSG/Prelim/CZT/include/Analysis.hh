#ifndef ANALYSIS_HH
#define ANALYSIS_HH

#include "BroadTree.hh"
#include "TreeModule.hh"
#include <TH1D.h>
#include <THStack.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TString.h>
#include <vector>

class Analysis {
public:
  // Inputs are <name>.root under the ROOT files base directory, or absolute
  // paths. Their broadened copies, <name>_broadened.root, go next to them, or
  // into broadenedDirectory if it is given.
  Analysis(const std::vector<TString> &inputNames,
           const std::vector<TString> &measurementTimes,
           const TString &broadenedDirectory = "");
  ~Analysis();
  void loadFiles(Bool_t broaden = kFALSE);
  void drawHists(const TString detectorName, Bool_t isNormed);
  void drawPartialHists(const TString detectorName, Double_t lowerBound,
                        Double_t upperBound, Bool_t isNormed, Int_t nbins);

  void fitGaussianToPeak(TLegend *legend, TH1D *hist, Double_t peak = 68.75,
                         Double_t range = 5, TString measurementTime = "");
  void ClearTHStack(THStack *stack);

private:
  TString getBroadenedName(const TString &inputName) const;
  void styleSpectra(std::vector<TH1D *> &spectra, Bool_t isNormed);

  std::vector<TString> inputNames;
  std::vector<TString> measurementTimes;
  TString broadenedDirectory;
  std::vector<BroadTree *> broadTrees;

  THStack *CZTStack;
  THStack *HPGeStack;
  THStack *SiLiStack;

  THStack *CZTStackPartial;
  THStack *HPGeStackPartial;
  THStack *SiLiStackPartial;
};

#endif
