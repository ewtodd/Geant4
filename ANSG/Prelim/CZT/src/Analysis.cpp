#include "Analysis.hh"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TF1.h>
#include <TList.h>
#include <TMath.h>
#include <TPad.h>
#include <TSystem.h>
#include <iostream>

Analysis::Analysis(const std::vector<TString> &names,
                   const std::vector<TString> &times,
                   const TString &directory) {
  inputNames = names;
  measurementTimes = times;
  broadenedDirectory = directory;
  if (measurementTimes.size() != inputNames.size()) {
    std::cerr << "Got " << measurementTimes.size()
              << " measurement times for " << inputNames.size()
              << " inputs; the missing ones are left blank." << std::endl;
    measurementTimes.resize(inputNames.size(), "");
  }
  CZTStack = new THStack();
  HPGeStack = new THStack();
  SiLiStack = new THStack();
  CZTStackPartial = new THStack();
  HPGeStackPartial = new THStack();
  SiLiStackPartial = new THStack();
}

Analysis::~Analysis() {
  Int_t nTrees = broadTrees.size();
  for (Int_t i = 0; i < nTrees; i++) {
    delete broadTrees.at(i);
  }
  THStack *stacks[6] = {CZTStack,        HPGeStack,        SiLiStack,
                        CZTStackPartial, HPGeStackPartial, SiLiStackPartial};
  for (Int_t i = 0; i < 6; i++) {
    ClearTHStack(stacks[i]);
    delete stacks[i];
  }
}

TString Analysis::getBroadenedName(const TString &inputName) const {
  if (broadenedDirectory == "")
    return inputName + "_broadened";
  return broadenedDirectory + "/" + gSystem->BaseName(inputName) +
         "_broadened";
}

// With broaden, writes each input's broadened trees first; either way, then
// opens the broadened files for drawing.
void Analysis::loadFiles(Bool_t broaden) {
  Int_t nInputs = inputNames.size();
  if (broaden) {
    for (Int_t i = 0; i < nInputs; i++) {
      TreeModule *treeModule =
          new TreeModule(inputNames.at(i) + ".root",
                         getBroadenedName(inputNames.at(i)) + ".root");
      treeModule->broadenAndStoreEnergy();
      // Closes the broadened file, so that it can be read below.
      delete treeModule;
    }
  }

  Int_t nTrees = broadTrees.size();
  for (Int_t i = 0; i < nTrees; i++) {
    delete broadTrees.at(i);
  }
  broadTrees.clear();
  for (Int_t i = 0; i < nInputs; i++) {
    broadTrees.push_back(
        new BroadTree(getBroadenedName(inputNames.at(i)) + ".root"));
  }
}

void Analysis::styleSpectra(std::vector<TH1D *> &spectra, Bool_t isNormed) {
  std::vector<Int_t> colors = PlottingUtils::GetDefaultColors();
  Int_t nColors = colors.size();
  Int_t nSpectra = spectra.size();
  for (Int_t i = 0; i < nSpectra; i++) {
    PlottingUtils::ConfigureHistogram(spectra.at(i), colors.at(i % nColors));
    if (isNormed)
      spectra.at(i)->Scale(1.0 / spectra.at(i)->Integral());
  }
}

void Analysis::drawHists(const TString detectorName, Bool_t isNormed) {
  THStack *stack = nullptr;
  TString fileName = detectorName;
  TString yTitle = "Counts";

  if (detectorName == "CZT") {
    stack = CZTStack;
  } else if (detectorName == "HPGe") {
    stack = HPGeStack;
  } else if (detectorName == "SiLi") {
    stack = SiLiStack;
  }
  if (!stack) {
    std::cerr << "Unknown detector " << detectorName << std::endl;
    return;
  }
  ClearTHStack(stack);

  std::vector<TH1D *> spectra;
  Int_t nTrees = broadTrees.size();
  for (Int_t i = 0; i < nTrees; i++) {
    spectra.push_back(broadTrees.at(i)->energySpectrumHist(detectorName));
  }
  styleSpectra(spectra, isNormed);
  if (isNormed) {
    fileName += "Normed";
    yTitle = "Counts / # of Incident Neutrons";
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  // The largest sample first, as before.
  for (Int_t i = nTrees - 1; i >= 0; i--) {
    stack->Add(spectra.at(i));
  }
  stack->Draw("nostack");
  stack->GetXaxis()->SetTitle("Energy [keV]");
  stack->GetYaxis()->SetTitle(yTitle);
  // Room for five-digit counts beside the y title.
  canvas->SetLeftMargin(0.18);
  stack->GetYaxis()->SetTitleOffset(1.5);

  PlottingUtils::SaveFigure(canvas, fileName, "", PlotSaveOptions::kLINEAR);
  delete canvas;
}

void Analysis::drawPartialHists(const TString detectorName, Double_t lowerBound,
                                Double_t upperBound, Bool_t isNormed,
                                Int_t nbins) {
  THStack *stackPartial = nullptr;
  TString yTitle = "Counts";

  if (detectorName == "CZT") {
    stackPartial = CZTStackPartial;
  } else if (detectorName == "HPGe") {
    stackPartial = HPGeStackPartial;
  } else if (detectorName == "SiLi") {
    stackPartial = SiLiStackPartial;
  }
  if (!stackPartial) {
    std::cerr << "Unknown detector " << detectorName << std::endl;
    return;
  }
  ClearTHStack(stackPartial);

  std::vector<TH1D *> spectra;
  Int_t nTrees = broadTrees.size();
  for (Int_t i = 0; i < nTrees; i++) {
    spectra.push_back(broadTrees.at(i)->energySpectrumHist(
        detectorName, lowerBound, upperBound, nbins));
  }
  styleSpectra(spectra, isNormed);

  TString fileName = Form("partialHist_%.2f_%.2f", lowerBound, upperBound);
  if (isNormed) {
    fileName += "Normed";
    yTitle = "Normalized Counts";
  }

  // The legend goes beside the plot, where it cannot hide the peak; its
  // width is measured on labels with a placeholder error.
  TString header = "68.75 keV Peak Position Errors";
  std::vector<TString> sampleLabels;
  for (Int_t i = 0; i < nTrees; i++) {
    sampleLabels.push_back(measurementTimes.at(i) + ": #pm 0.000 keV");
  }
  const Double_t gapPx = 20;
  const Double_t rowPx = 45;
  Double_t legendWidthPx = PlottingUtils::LegendWidthPx(sampleLabels, header);
  TPad *plot = nullptr;
  TPad *legendPad = nullptr;
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvasWithSideLegend(
      plot, legendPad, TMath::CeilNint(legendWidthPx + 2 * gapPx), 800, kTRUE);

  plot->cd();
  for (Int_t i = nTrees - 1; i >= 0; i--) {
    stackPartial->Add(spectra.at(i));
  }
  stackPartial->Draw("nostack, HIST");
  stackPartial->GetXaxis()->SetTitle("Energy [keV]");
  stackPartial->GetYaxis()->SetTitle(yTitle);

  legendPad->cd();
  Double_t legendPadWidthPx = legendPad->GetWNDC() * canvas->GetWw();
  Double_t legendTop = 1 - plot->GetTopMargin();
  TLegend *legend = PlottingUtils::AddLegend(
      gapPx / legendPadWidthPx, (gapPx + legendWidthPx) / legendPadWidthPx,
      legendTop - (nTrees + 1) * rowPx / canvas->GetWh(), legendTop);
  legend->SetHeader(header, "C");
  for (Int_t i = 0; i < nTrees; i++) {
    fitGaussianToPeak(legend, spectra.at(i), 68.75, 5, measurementTimes.at(i));
  }

  // Log y is set on the plot pad itself, so the figure is saved as it is.
  PlottingUtils::SaveFigure(canvas, fileName + detectorName, "",
                            PlotSaveOptions::kLINEAR);
  delete canvas;
}

void Analysis::ClearTHStack(THStack *stack) {
  TList *histList = stack->GetHists();
  if (!histList)
    return;
  while (histList->GetSize() > 0) {
    TObject *obj = histList->First();
    histList->Remove(obj);
    delete obj;
  }
}

void Analysis::fitGaussianToPeak(TLegend *legend, TH1D *hist, Double_t peak,
                                 Double_t range, TString measurementTime) {
  Double_t fitRangeMin = peak - range;
  Double_t fitRangeMax = peak + range;
  TF1 *gaussFit = new TF1(PlottingUtils::GetRandomName(),
                          "gaus + [3]*x*x + [4]*x + [5]", fitRangeMin,
                          fitRangeMax);

  // Initial parameters: [0] = height, [1] = mean, [2] = sigma
  gaussFit->SetParameters(hist->GetMaximum(), peak, 1.0);
  gaussFit->SetLineColor(hist->GetLineColor());
  gaussFit->SetLineWidth(hist->GetLineWidth());
  // "0": the spectra are drawn as HIST, which never shows the fit anyway.
  hist->Fit(gaussFit, "RQ0");
  Double_t peakMean = gaussFit->GetParameter(1);
  Double_t peakMeanError = gaussFit->GetParError(1);
  legend->AddEntry(gaussFit,
                   measurementTime + ": " +
                       TString::Format("#pm %.3f keV", peakMeanError),
                   "l");

  std::cout << "Peak position (mean): " << peakMean << " +/- " << peakMeanError
            << " keV" << std::endl;
}
