#include "PlotFunctions.cpp"
#include <TCanvas.h>
#include <TFile.h>
#include <TGaxis.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TTree.h>
#include <iostream>
#include <sstream>
#include <vector>

// Function to extract data from the histogram and write to Geant4 macro file
void analyzeAndPlotSpectra() {

  gROOT->SetBatch(kTRUE);
  // Define thermal energy range in MeV (1 eV = 1e-6 MeV) for thermal counting
  Double_t thermalEnergyLower = 0.015 * 1e-6; // 0.01 eV in MeV
  Double_t thermalEnergyUpper = 0.030 * 1e-6; // 0.05 eV in MeV

  // Arrays to store the thicknesses and corresponding normalized thermal counts
  std::vector<double> thicknesses;
  std::vector<double> waterThermalCounts;
  std::vector<double> polyThermalCounts;

  // Function to process ROOT files and extract thermal counts
  auto processFiles = [&](const std::string &material,
                          std::vector<double> &thermalCounts) {
    for (int thickness = 10; thickness <= 17; thickness += 1) {
      // Create the filename
      std::ostringstream filename;
      filename << "../" << material << thickness << "cm.root";

      // Open the ROOT file
      TFile *file = TFile::Open(filename.str().c_str());

      if (!file || file->IsZombie()) {
        std::cerr << "Error opening file " << filename.str() << "!"
                  << std::endl;
        continue;
      }

      // Get the ntuple from the file
      TTree *tree = (TTree *)file->Get("PostEnergy");

      if (!tree) {
        std::cerr << "Error: PostEnergy tree not found in " << filename.str()
                  << "!" << std::endl;
        file->Close();
        delete file; // ensure file is deleted
        continue;
      }

      // Variables to hold the branch values
      Double_t postEnergy;

      // Set branch addresses
      tree->SetBranchAddress("fPostEnergy", &postEnergy);

      Int_t nEntries = tree->GetEntries();
      Int_t thermalCount = 0;

      // Loop over tree entries to count thermal neutrons
      for (Int_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        if (postEnergy >= thermalEnergyLower &&
            postEnergy <= thermalEnergyUpper) {
          ++thermalCount;
        }
      }

      // Normalize the thermal count
      double normalizedThermalCount = static_cast<double>(thermalCount) * 100;

      // Store the results
      // Only add the thickness once
      if (material == "Water") {
        thicknesses.push_back(thickness);
      }
      thermalCounts.push_back(normalizedThermalCount);

      // Output results for current file
      std::cout << material << " File: " << filename.str() << std::endl;
      std::cout << "  Total entries: " << nEntries << std::endl;
      std::cout << "  Number of thermal neutrons: " << thermalCount
                << std::endl;
      std::cout << "  Normalized number of thermal neutrons: "
                << normalizedThermalCount << std::endl;

      // Clean up
      file->Close();
      delete file;
    }
  };

  // Process water files
  processFiles("Water", waterThermalCounts);

  // Process poly files
  processFiles("Poly", polyThermalCounts);

  // Create a canvas to plot the thermal neutrons vs. thickness
  TCanvas *c3 =
      new TCanvas("c3", "Thermal Neutrons vs. Moderator Thickness", 2000, 1600);
  // Create the TGraph for poly files
  // Create TGraphs for each material
  TGraph *graphWater =
      new TGraph(thicknesses.size(), &thicknesses[0], &waterThermalCounts[0]);
  graphWater->SetTitle(";Moderator Thickness "
                       "(cm);Estimated Thermal Neutron Flux (n/s)");
  graphWater->SetMarkerStyle(20);
  graphWater->SetMarkerSize(2);
  graphWater->SetMarkerColor(kBlue);

  // Create the TGraph for poly files
  TGraph *graphPoly =
      new TGraph(thicknesses.size(), &thicknesses[0], &polyThermalCounts[0]);
  graphPoly->SetMarkerStyle(21);
  graphPoly->SetMarkerSize(2);
  graphPoly->SetMarkerColor(kRed);

  // Draw the graphs
  graphWater->Draw("AP");
  graphPoly->Draw("P SAME"); // Create a legend for the plot

  // Find the min and max y-values for both graphs
  double minY = std::min(
      *std::min_element(waterThermalCounts.begin(), waterThermalCounts.end()),
      *std::min_element(polyThermalCounts.begin(), polyThermalCounts.end()));
  double maxY = std::max(
      *std::max_element(waterThermalCounts.begin(), waterThermalCounts.end()),
      *std::max_element(polyThermalCounts.begin(), polyThermalCounts.end()));
  // Add this immediately after setting minY and maxY
  if (minY <= 0) {
    minY = 1e-3; // Set minimum to a small positive value if zero/negative
  }
  // Set the range for Y-axis to make it common for both graphs
  graphWater->GetHistogram()->SetMinimum(minY * 0.9); // Slightly lower minimum
  graphWater->GetHistogram()->SetMaximum(maxY * 1.1); // Slightly higher maximum

  // Create the main title and move it down slightly
  TPaveText *title = new TPaveText(0.1, 0.93, 0.9, 0.98, "NDC");
  title->AddText("Thermal Neutron Flux from Moderated DT Generator");
  title->SetFillColor(0);  // Set the background color
  title->SetTextAlign(22); // Center align the text
  title->SetTextFont(42);  // Set the font
  title->Draw();

  TLegend *legend = new TLegend(0.72, 0.19, 0.83, 0.3);
  legend->AddEntry(graphWater, "Water", "p");
  legend->AddEntry(graphPoly, "HDP", "p");
  legend->SetTextSize(0.03); // Set smaller text size for the legend
  legend->Draw();
  SetCanvasMargins(c3);
  graphWater->GetYaxis()->SetTitleOffset(2); // Adjust the offset if needed
                                             // add additional ticks
  c3->SetTickx();
  c3->SetTicky();
  TAxis *axis = graphWater->GetYaxis();
  gPad->SetLogy();
  axis->SetMoreLogLabels();
  // add grid lines
  c3->SetGridx();
  c3->SetGridy();

  // Save the thermal neutrons vs thickness plot
  c3->SaveAs("ThermalNeutronsVsThickness.png");
  // Additional: Generate full and thermal energy spectrum plots for 11cm Poly
  // and 15cm Water
  std::vector<std::pair<std::string, int>> specialFiles = {{"Poly", 11},
                                                           {"Water", 15}};
  TH1D *thermalEnergyHistPoly =
      new TH1D("ThermalEnergyHistPoly",
               ";Energy (eV); "
               "Normalized Counts",
               50, 0, 0.15); // Adjusted bin width to 0.1 eV
  TH1D *thermalEnergyHistWater =
      new TH1D("ThermalEnergyHistWater",
               ";Energy (eV); "
               "Normalized Counts",
               50, 0, 0.15); // Adjusted bin width to 0.1 eV
  for (const auto &file : specialFiles) {
    std::ostringstream filename;
    filename << "../" << file.first << file.second << "cm.root";
    TFile *specFile = TFile::Open(filename.str().c_str());

    if (!specFile || specFile->IsZombie()) {
      std::cerr << "Error opening file " << filename.str() << "!" << std::endl;
      if (specFile)
        delete specFile;
      continue;
    }

    TTree *tree = (TTree *)specFile->Get("PostEnergy");

    if (!tree) {
      std::cerr << "Error: PostEnergy tree not found in " << filename.str()
                << "!" << std::endl;
      specFile->Close();
      delete specFile; // ensure file is deleted
      continue;
    }

    Double_t postEnergy;
    tree->SetBranchAddress("fPostEnergy", &postEnergy);
    Int_t nEntries = tree->GetEntries();

    for (Int_t i = 0; i < nEntries; ++i) {
      tree->GetEntry(i);
      if (file.first == "Poly") {
        if (postEnergy >= (0.001 * 1e-6) && postEnergy <= (0.15 * 1e-6)) {
          thermalEnergyHistPoly->Fill(postEnergy * 1e6); // Convert to eV
        }
      } else if (file.first == "Water") {
        if (postEnergy >= (0.001 * 1e-6) && postEnergy <= (0.15 * 1e-6)) {
          thermalEnergyHistWater->Fill(postEnergy * 1e6); // Convert to eV
        }
      }
    }

    specFile->Close();
    delete specFile;
  }

  if (thermalEnergyHistPoly->Integral() > 0) {
    thermalEnergyHistPoly->Scale(1.0 / thermalEnergyHistPoly->Integral());
  }
  if (thermalEnergyHistWater->Integral() > 0) {
    thermalEnergyHistWater->Scale(1.0 / thermalEnergyHistWater->Integral());
  }

  // Save the thermal energy histograms to a ROOT file
  TFile *outputFile = TFile::Open("thermalEnergySpectra.root", "RECREATE");
  if (outputFile && outputFile->IsOpen()) {
    thermalEnergyHistWater->Write();
    outputFile->Close();
    delete outputFile;
  } else {
    std::cerr << "Error creating ROOT file for thermal energy spectra!"
              << std::endl;
  }

  // Draw Thermal Energy Spectrum
  TCanvas *thermalEnergyCanvas =
      new TCanvas("ThermalEnergyCanvas", "Thermal Energy Spectrum", 1600, 1200);
  thermalEnergyHistPoly->SetLineColor(kRed);
  thermalEnergyHistPoly->SetStats(0); // Hide stats
  thermalEnergyHistWater->SetLineColor(kBlue);
  thermalEnergyHistWater->SetStats(0); // Hide stats
  thermalEnergyHistWater->SetLineWidth(2);
  thermalEnergyHistPoly->SetLineWidth(2);
  thermalEnergyHistWater->Draw("HIST");
  thermalEnergyHistPoly->Draw("HIST SAME");
  TLegend *thermalEnergyLegend = new TLegend(0.7, 0.7, 0.9, 0.9);
  thermalEnergyLegend->AddEntry(thermalEnergyHistWater, "Water, 15 cm", "l");
  thermalEnergyLegend->AddEntry(thermalEnergyHistPoly, "HDP, 11 cm", "l");
  thermalEnergyLegend->Draw();
  // Create the main title and move it down slightly
  TPaveText *titleThermal = new TPaveText(0.1, 0.93, 0.9, 0.98, "NDC");
  titleThermal->AddText("Moderated DT Generator Thermal Neutron Spectrum");
  titleThermal->SetFillColor(0);  // Set the background color
  titleThermal->SetTextAlign(22); // Center align the text
  titleThermal->SetTextFont(42);  // Set the font
  titleThermal->Draw();
  // add additional ticks
  thermalEnergyCanvas->SetTickx();
  thermalEnergyCanvas->SetTicky();

  // add grid lines
  thermalEnergyCanvas->SetGridx();
  thermalEnergyCanvas->SetGridy();

  SetCanvasMargins(thermalEnergyCanvas);
  thermalEnergyCanvas->SaveAs("ThermalEnergySpectrum.png");

  // Cleanup
  delete thermalEnergyHistPoly;
  delete thermalEnergyHistWater;
  delete thermalEnergyLegend;
  delete thermalEnergyCanvas;
  delete graphWater;
  delete graphPoly;
  delete legend;
}
