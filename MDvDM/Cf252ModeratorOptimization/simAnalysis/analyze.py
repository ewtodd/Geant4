import numpy as np
import matplotlib.pyplot as plt
import uproot
import os
import matplotlib as mpl
import mplhep as hep

mpl.use('Agg')
hep.style.use(hep.style.ROOT)
plt.rcParams['figure.figsize'] = [14, 10]
plt.rcParams['legend.facecolor'] = 'white'    # Legend background color
plt.rcParams['legend.framealpha'] = 1         # Legend background opacity (1 = opaque, 0 = transparent)
plt.rcParams['legend.edgecolor'] = 'black'    # Legend border color
plt.rcParams['legend.fancybox'] = True        # Rounded box corners (optional, default True)
plt.rcParams['legend.frameon'] = True         # Show legend frame (default True)
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Liberation Serif']


def ansg(text="Simulation"):
    hep.label.exp_text(
    exp="ANSG",  
    text=text,   
    loc=0,                
    italic=(False,True,False),
    fontname=plt.rcParams['font.serif'][0]
    )


# Define thermal energy range in MeV (1 eV = 1e-6 MeV) for thermal counting
thermal_energy_lower = 0.015 * 1e-6  # 0.01 eV in MeV
thermal_energy_upper = 0.030 * 1e-6  # 0.05 eV in MeV

# Arrays to store the thicknesses and corresponding normalized thermal counts
thicknesses = []
poly_thermal_counts = []

# Function to process ROOT files and extract thermal counts
def process_files(material, thermal_counts):
    for thickness in range(5, 13):
        # Create the filename
        filename = f"../{material}{thickness}cm.root"
        if not os.path.exists(filename):
            print(f"Error: file {filename} does not exist!")
            continue

        # Open the ROOT file
        with uproot.open(filename) as file:
            # Get the ntuple from the file
            tree = file.get("PostEnergy")
            if tree is None:
                print(f"Error: PostEnergy tree not found in {filename}!")
                continue

            # Extract branch values
            post_energies = tree["fPostEnergy"].array(library="np")
            # Count thermal neutrons
            thermal_count = np.sum((post_energies >= thermal_energy_lower) & (post_energies <= thermal_energy_upper))
            # Normalize the count
            normalized_thermal_count = thermal_count / 1e6

            # Store the results
            if material == "Poly":
                thicknesses.append(thickness)
                thermal_counts.append(normalized_thermal_count)

            print(f"{material} File: {filename}")
            print(f"  Total entries: {len(post_energies)}")
            print(f"  Number of thermal neutrons: {thermal_count}")
            print(f"  Normalized number of thermal neutrons: {normalized_thermal_count}")

# Process poly files only
process_files("Poly", poly_thermal_counts)

# Plotting using Matplotlib
plt.figure()
plt.plot(thicknesses, poly_thermal_counts, '.-', color='red')
ansg()
plt.yscale('log')
plt.xlabel('HDPE Thickness [cm]')
plt.ylabel('Thermal Neutrons Per Incident Cf-252 Neutron')
plt.grid()
ansg()
plt.savefig('ThermalNeutronsVsThickness.png')

# Generate full and thermal energy spectrum plots for 11cm Poly only
special_files = [("Poly", 8)]
filename = f"../Poly8cm.root"
if not os.path.exists(filename):
    print(f"Error: file {filename} does not exist!")
    
selected_energies=[]
with uproot.open(filename) as file:
    tree = file.get("PostEnergy")
    if tree is None:
        print(f"Error: PostEnergy tree not found in {filename}!")
    post_energies = tree["fPostEnergy"].array(library="np") * 1e6  # Convert to eV
    selected_energies = post_energies[(post_energies >= 0.0001) & (post_energies <= 0.15)]
energy_hist, bins = np.histogram(selected_energies, bins=250, range=(0, 0.15))
energy_hist = energy_hist/energy_hist.sum()
# Plot the energy histogram
plt.figure()
hep.histplot(energy_hist,bins, histtype="fill",label='Cf-252 + 8 cm HDPE Moderator Neutron Spectrum', alpha=0.5,color='red')
hep.histplot(energy_hist,bins, histtype="step",color='red')
plt.xlabel('Energy [eV]')
plt.ylabel('Normalized Distribution')
plt.grid()
ansg()
plt.legend(loc='upper right')
plt.savefig('ThermalEnergySpectrum.png')
