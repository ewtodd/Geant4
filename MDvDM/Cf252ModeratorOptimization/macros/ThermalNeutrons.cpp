#include "Constants.hpp"
#include "IOUtils.hpp"
#include "InitUtils.hpp"
#include "PlottingUtils.hpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TTree.h>
#include <iostream>
#include <vector>

TString ModeratorFileName(const TString &input_prefix, Int_t thickness_cm) {
  return Form("%s%dcm.root", input_prefix.Data(), thickness_cm);
}

// The caller owns the file and must close and delete it; null if unusable.
TFile *OpenModeratorFile(const TString &filename) {
  TFile *file = IO::OpenForReading(filename);
  if (file->IsZombie()) {
    std::cerr << "Error: file " << filename << " does not exist!" << std::endl;
    delete file;
    return nullptr;
  }
  if (!file->Get(Constants::POST_TREE)) {
    std::cerr << "Error: " << Constants::POST_TREE << " tree not found in "
              << filename << "!" << std::endl;
    file->Close();
    delete file;
    return nullptr;
  }
  return file;
}

TTree *GetPostEnergyTree(TFile *file, Double_t &post_energy_mev) {
  TTree *tree = static_cast<TTree *>(file->Get(Constants::POST_TREE));
  tree->SetBranchStatus("*", 0);
  tree->SetBranchStatus(Constants::POST_ENERGY_BRANCH, 1);
  tree->SetBranchAddress(Constants::POST_ENERGY_BRANCH, &post_energy_mev);
  return tree;
}

void PlotThermalNeutronsVsThickness(const TString &input_prefix,
                                    Double_t incident_neutrons) {
  std::vector<Double_t> thicknesses_cm;
  std::vector<Double_t> thermal_per_neutron;

  for (Int_t thickness_cm = Constants::THICKNESS_MIN_CM;
       thickness_cm <= Constants::THICKNESS_MAX_CM; thickness_cm++) {
    TString filename = ModeratorFileName(input_prefix, thickness_cm);
    TFile *file = OpenModeratorFile(filename);
    if (!file)
      continue;

    Double_t post_energy_mev;
    TTree *tree = GetPostEnergyTree(file, post_energy_mev);

    Long64_t n_entries = tree->GetEntries();
    Long64_t thermal_count = 0;
    for (Long64_t i = 0; i < n_entries; i++) {
      tree->GetEntry(i);
      if (post_energy_mev >= Constants::THERMAL_ENERGY_LOWER_MEV &&
          post_energy_mev <= Constants::THERMAL_ENERGY_UPPER_MEV)
        thermal_count++;
    }
    Double_t normalized_thermal_count = thermal_count / incident_neutrons;

    thicknesses_cm.push_back(thickness_cm);
    thermal_per_neutron.push_back(normalized_thermal_count);

    std::cout << "File: " << file->GetName() << std::endl;
    std::cout << "  Total entries: " << n_entries << std::endl;
    std::cout << "  Number of thermal neutrons: " << thermal_count
              << std::endl;
    std::cout << "  Normalized number of thermal neutrons: "
              << normalized_thermal_count << std::endl;

    file->Close();
    delete file;
  }

  Int_t n_points = thicknesses_cm.size();
  if (n_points == 0) {
    std::cerr << "No moderator files found; ThermalNeutronsVsThickness not "
                 "plotted."
              << std::endl;
    return;
  }

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  // On one line the y title is taller than the frame; two need the room.
  canvas->SetLeftMargin(0.2);
  TGraph *graph = new TGraph(n_points, thicknesses_cm.data(),
                             thermal_per_neutron.data());
  PlottingUtils::ConfigureGraph(graph, kRed,
                                ";HDPE Thickness [cm];#splitline{Thermal "
                                "Neutrons Per}{Incident Cf-252 Neutron}");
  graph->GetYaxis()->SetTitleOffset(1.4);
  graph->GetXaxis()->SetLimits(Constants::THICKNESS_MIN_CM - 0.5,
                               Constants::THICKNESS_MAX_CM + 0.5);
  graph->SetMarkerStyle(20);
  graph->SetMarkerColor(kRed);
  graph->Draw("APL");
  PlottingUtils::AddText(Constants::ANSG_LABEL, Constants::ANSG_LABEL_X,
                         Constants::ANSG_LABEL_Y);

  PlottingUtils::SaveFigure(canvas, "ThermalNeutronsVsThickness", "",
                            PlotSaveOptions::kLOG);

  delete canvas;
  delete graph;
}

void PlotThermalEnergySpectrum(const TString &input_prefix,
                               Int_t thickness_cm) {
  TString filename = ModeratorFileName(input_prefix, thickness_cm);
  TFile *file = OpenModeratorFile(filename);
  if (!file)
    return;

  Double_t post_energy_mev;
  TTree *tree = GetPostEnergyTree(file, post_energy_mev);

  TH1D *spectrum =
      new TH1D(PlottingUtils::GetRandomName(), "", Constants::SPECTRUM_NBINS,
               Constants::SPECTRUM_XMIN_EV, Constants::SPECTRUM_XMAX_EV);

  Long64_t n_entries = tree->GetEntries();
  for (Long64_t i = 0; i < n_entries; i++) {
    tree->GetEntry(i);
    Double_t post_energy_ev = post_energy_mev * Constants::MEV_TO_EV;
    if (post_energy_ev >= Constants::SPECTRUM_SELECTION_MIN_EV &&
        post_energy_ev <= Constants::SPECTRUM_XMAX_EV)
      spectrum->Fill(post_energy_ev);
  }
  file->Close();
  delete file;

  if (spectrum->Integral() > 0)
    spectrum->Scale(1.0 / spectrum->Integral());

  TCanvas *canvas = PlottingUtils::GetConfiguredCanvas();
  PlottingUtils::ConfigureHistogram(spectrum, kRed,
                                    ";Energy [eV];Normalized Distribution");
  spectrum->SetFillColorAlpha(kRed, 0.5);
  spectrum->SetFillStyle(1001);
  spectrum->Draw("HIST");

  // On one line the label is wider than the frame.
  TString label_first_line = Form("Cf-252 + %d cm HDPE", thickness_cm);
  TString label_second_line = "Moderator Neutron Spectrum";
  std::vector<TString> label_lines = {label_first_line, label_second_line};
  Double_t legend_width =
      PlottingUtils::LegendWidthPx(label_lines) / canvas->GetWw();
  TLegend *legend =
      PlottingUtils::AddLegend(0.88 - legend_width, 0.88, 0.72, 0.86);
  legend->AddEntry(spectrum,
                   "#splitline{" + label_first_line + "}{" +
                       label_second_line + "}",
                   "f");
  PlottingUtils::AddText(Constants::ANSG_LABEL, Constants::ANSG_LABEL_X,
                         Constants::ANSG_LABEL_Y);

  PlottingUtils::SaveFigure(canvas, "ThermalEnergySpectrum", "",
                            PlotSaveOptions::kLINEAR);

  delete canvas;
  delete spectrum;
}

// Thermal (0.015 to 0.030 eV) neutrons behind HDPE moderators of 5 to 12 cm,
// per incident Cf-252 neutron, and the spectrum behind one of them.
void ThermalNeutrons(TString input_prefix = Constants::MODERATOR_PREFIX,
                     Int_t spectrum_thickness_cm =
                         Constants::SPECTRUM_THICKNESS_CM,
                     Double_t incident_neutrons = Constants::INCIDENT_NEUTRONS) {
  InitUtils::SetROOTPreferences();

  PlotThermalNeutronsVsThickness(input_prefix, incident_neutrons);
  PlotThermalEnergySpectrum(input_prefix, spectrum_thickness_cm);
}
