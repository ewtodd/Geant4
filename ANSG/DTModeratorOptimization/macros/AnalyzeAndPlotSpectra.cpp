#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TMath.h>
#include <TROOT.h>
#include <TTree.h>
#include <vector>

struct ThermalCounts {
  std::vector<Double_t> thicknesses_cm;
  std::vector<Double_t> scaled_counts;
};

TString InputName(const TString input_prefix, const TString material,
                  Int_t thickness_cm) {
  return Form("%s%s%dcm", input_prefix.Data(), material.Data(), thickness_cm);
}

// Returns the PostEnergy tree of <name>.root, or null with file closed.
TTree *OpenPostEnergyTree(const TString name, TFile *&file) {
  file = IO::OpenForReading(name + ".root");
  if (file->IsZombie()) {
    std::cerr << "Error opening file " << name << ".root!" << std::endl;
    delete file;
    file = nullptr;
    return nullptr;
  }
  TTree *tree = static_cast<TTree *>(file->Get("PostEnergy"));
  if (!tree) {
    std::cerr << "Error: PostEnergy tree not found in " << name << ".root!"
              << std::endl;
    file->Close();
    delete file;
    file = nullptr;
  }
  return tree;
}

ThermalCounts CountThermalNeutrons(const TString input_prefix,
                                   const TString material) {
  ThermalCounts counts;
  for (Int_t thickness = Constants::THICKNESS_MIN_CM;
       thickness <= Constants::THICKNESS_MAX_CM; thickness++) {
    TString name = InputName(input_prefix, material, thickness);
    TFile *file = nullptr;
    TTree *tree = OpenPostEnergyTree(name, file);
    if (!tree)
      continue;

    Double_t post_energy_MeV;
    tree->SetBranchAddress("fPostEnergy", &post_energy_MeV);
    Long64_t n_entries = tree->GetEntries();
    Long64_t thermal_count = 0;
    for (Long64_t i = 0; i < n_entries; i++) {
      tree->GetEntry(i);
      if (post_energy_MeV >= Constants::THERMAL_ENERGY_LOWER_MEV &&
          post_energy_MeV <= Constants::THERMAL_ENERGY_UPPER_MEV)
        thermal_count++;
    }
    file->Close();
    delete file;

    Double_t scaled_count = thermal_count * Constants::THERMAL_COUNT_SCALE;
    counts.thicknesses_cm.push_back(thickness);
    counts.scaled_counts.push_back(scaled_count);

    std::cout << material << " File: " << name << ".root" << std::endl;
    std::cout << "  Total entries: " << n_entries << std::endl;
    std::cout << "  Number of thermal neutrons: " << thermal_count
              << std::endl;
    std::cout << "  Normalized number of thermal neutrons: " << scaled_count
              << std::endl;
  }
  return counts;
}

void PlotThermalFluxVsThickness(const ThermalCounts &water,
                                const ThermalCounts &poly) {
  std::vector<const ThermalCounts *> series = {&water, &poly};
  std::vector<TString> labels = {"Water", "HDP"};
  std::vector<Int_t> colors = {kBlue, kRed};
  std::vector<Int_t> marker_styles = {20, 21};

  std::vector<Double_t> all_counts;
  all_counts.insert(all_counts.end(), water.scaled_counts.begin(),
                    water.scaled_counts.end());
  all_counts.insert(all_counts.end(), poly.scaled_counts.begin(),
                    poly.scaled_counts.end());
  if (all_counts.empty()) {
    std::cerr << "No moderator files were read; no flux plot." << std::endl;
    return;
  }
  Double_t min_count = TMath::MinElement(all_counts.size(), all_counts.data());
  Double_t max_count = TMath::MaxElement(all_counts.size(), all_counts.data());
  // The y axis is logarithmic.
  if (min_count <= 0)
    min_count = 1e-3;

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  canvas->SetLeftMargin(0.2);
  TLegend *legend = nullptr;
  std::vector<TGraph *> graphs;
  Int_t n_series = series.size();
  for (Int_t i = 0; i < n_series; i++) {
    const ThermalCounts *counts = series.at(i);
    Int_t n_points = counts->thicknesses_cm.size();
    if (n_points == 0)
      continue;

    TGraph *graph = new TGraph(n_points, counts->thicknesses_cm.data(),
                               counts->scaled_counts.data());
    PlottingUtils::ConfigureGraph(
        graph, colors.at(i),
        "Thermal Neutron Flux from Moderated DT Generator; Moderator "
        "Thickness [cm]; Estimated Thermal Neutron Flux [n/s]");
    graph->SetMarkerStyle(marker_styles.at(i));
    graph->SetMarkerSize(2);
    graph->SetMarkerColor(colors.at(i));

    if (graphs.empty()) {
      graph->GetHistogram()->SetMinimum(min_count * 0.9);
      graph->GetHistogram()->SetMaximum(max_count * 1.1);
      graph->GetYaxis()->SetMoreLogLabels();
      // The y title is taller than the pad at the usual size.
      graph->GetYaxis()->SetTitleSize(0.05);
      graph->GetYaxis()->CenterTitle();
      graph->GetYaxis()->SetTitleOffset(1.5);
      graph->Draw("AP");
      legend = PlottingUtils::AddLegend(0.7, 0.88, 0.2, 0.35);
    } else {
      graph->Draw("P");
    }
    legend->AddEntry(graph, labels.at(i), "p");
    graphs.push_back(graph);
  }

  PlottingUtils::SaveFigure(canvas, "ThermalNeutronsVsThickness", "",
                            PlotSaveOptions::kLOG);
  delete canvas;
  Int_t n_graphs = graphs.size();
  for (Int_t i = 0; i < n_graphs; i++) {
    delete graphs.at(i);
  }
}

// Unit-normalized spectrum of the neutrons leaving one moderator, or an empty
// histogram if its file cannot be read.
TH1D *FillThermalSpectrum(const TString input_prefix, const TString material,
                          Int_t thickness_cm, const TString hist_name) {
  TH1D *hist = new TH1D(hist_name, "; Energy [eV]; Normalized Counts",
                        Constants::SPECTRUM_NBINS, Constants::SPECTRUM_XMIN_EV,
                        Constants::SPECTRUM_XMAX_EV);
  TFile *file = nullptr;
  TTree *tree =
      OpenPostEnergyTree(InputName(input_prefix, material, thickness_cm), file);
  if (!tree)
    return hist;

  Double_t post_energy_MeV;
  tree->SetBranchAddress("fPostEnergy", &post_energy_MeV);
  Long64_t n_entries = tree->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    if (post_energy_MeV >= Constants::SPECTRUM_LOWER_MEV &&
        post_energy_MeV <= Constants::SPECTRUM_UPPER_MEV)
      hist->Fill(post_energy_MeV * 1e6);
  }
  file->Close();
  delete file;

  if (hist->Integral() > 0)
    hist->Scale(1.0 / hist->Integral());
  return hist;
}

void WriteWaterSpectrum(TH1D *water_spectrum, const TString output_name) {
  // Never replace a good spectrum with an empty one.
  if (water_spectrum->GetEntries() == 0) {
    std::cerr << "The water spectrum is empty; " << output_name
              << ".root is not written." << std::endl;
    return;
  }
  TFile *output = IO::OpenForWriting(output_name + ".root");
  if (output->IsZombie()) {
    std::cerr << "Error creating ROOT file for thermal energy spectra!"
              << std::endl;
    delete output;
    return;
  }
  water_spectrum->Write(Constants::WATER_SPECTRUM_HIST_NAME,
                        TObject::kOverwrite);
  output->Close();
  delete output;
  std::cout << "Wrote " << Constants::WATER_SPECTRUM_HIST_NAME << " to "
            << output_name << ".root" << std::endl;
}

void PlotThermalSpectra(TH1D *water_spectrum, TH1D *poly_spectrum) {
  if (water_spectrum->GetEntries() == 0 && poly_spectrum->GetEntries() == 0) {
    std::cerr << "Both spectra are empty; no spectrum plot." << std::endl;
    return;
  }
  TString title = "Moderated DT Generator Thermal Neutron Spectrum; Energy "
                  "[eV]; Normalized Counts";
  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(water_spectrum, kBlue, title);
  PlottingUtils::ConfigureHistogram(poly_spectrum, kRed, title);
  // So that neither spectrum is clipped by the other's frame.
  water_spectrum->SetMaximum(
      1.05 * TMath::Max(water_spectrum->GetMaximum(),
                        poly_spectrum->GetMaximum()));
  water_spectrum->Draw("HIST");
  poly_spectrum->Draw("HIST SAME");

  TLegend *legend = PlottingUtils::AddLegend(0.62, 0.88, 0.65, 0.85);
  legend->AddEntry(water_spectrum,
                   Form("Water, %d cm", Constants::SPECTRUM_WATER_THICKNESS_CM),
                   "l");
  legend->AddEntry(poly_spectrum,
                   Form("HDP, %d cm", Constants::SPECTRUM_POLY_THICKNESS_CM),
                   "l");

  PlottingUtils::SaveFigure(canvas, "ThermalEnergySpectrum", "",
                            PlotSaveOptions::kLINEAR);
  delete canvas;
}

// input_prefix goes in front of each <material><thickness>cm name, so an
// absolute directory reads the inputs from there instead of root_files/.
void AnalyzeAndPlotSpectra(
    const TString input_prefix = "",
    const TString spectra_output_name = Constants::THERMAL_SPECTRA_OUTPUT) {
  InitUtils::SetROOTPreferences();

  ThermalCounts water = CountThermalNeutrons(input_prefix, Constants::WATER);
  ThermalCounts poly = CountThermalNeutrons(input_prefix, Constants::POLY);
  PlotThermalFluxVsThickness(water, poly);

  TH1D *poly_spectrum = FillThermalSpectrum(
      input_prefix, Constants::POLY, Constants::SPECTRUM_POLY_THICKNESS_CM,
      "ThermalEnergyHistPoly");
  TH1D *water_spectrum = FillThermalSpectrum(
      input_prefix, Constants::WATER, Constants::SPECTRUM_WATER_THICKNESS_CM,
      Constants::WATER_SPECTRUM_HIST_NAME);
  WriteWaterSpectrum(water_spectrum, spectra_output_name);
  PlotThermalSpectra(water_spectrum, poly_spectrum);

  delete poly_spectrum;
  delete water_spectrum;
}
