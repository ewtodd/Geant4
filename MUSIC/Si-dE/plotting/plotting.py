import ROOT
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import os

mpl.use('Agg')
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Liberation Serif']
plt.rcParams['font.size'] = 12
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12
plt.rcParams['lines.linewidth'] = 2
plt.rcParams['grid.linestyle'] = '--'
plt.rcParams['grid.alpha'] = 0.3
plt.rcParams['grid.color'] = 'black'
plt.rcParams['figure.figsize'] = [10, 8]
plt.rcParams['figure.autolayout'] = True
plt.rcParams['axes.linewidth'] = 1.5
plt.rcParams['xtick.major.width'] = 1.5
plt.rcParams['ytick.major.width'] = 1.5
plt.rcParams['xtick.direction'] = 'in'
plt.rcParams['ytick.direction'] = 'in'

# Define file pattern and parameters
distances = [ '2_5' ]
angles = [0, 30, 60]
file_pattern = "../air_{0}cm_{1}deg.root"

# Function to apply Gaussian energy broadening
def apply_gaussian_broadening(energy_values, fwhm_at_ref=16, ref_energy=5486):
    # Convert FWHM to sigma
    factor = fwhm_at_ref / ref_energy
    
    broadened_values = []
    for e in energy_values:
        sigma = (factor*e)/2.35 
        broadened_e = np.random.normal(e, sigma)
        broadened_values.append(broadened_e)
    
    return broadened_values

# Function to process data and create plot
def create_plot(apply_broadening=False, output_filename=None):
    plt.figure(figsize=(16, 10))
    colormap = plt.get_cmap('tab10')
    colors = [colormap(i) for i in range(len(distances)*len(angles))]
    
    color_index = 0
    
    for dist in distances:
        for angle in angles:
            filename = file_pattern.format(dist, angle)
            if not os.path.exists(filename):
                print(f"Warning: File {filename} not found. Skipping.")
                continue
               
            root_file = ROOT.TFile.Open(filename)
            tree = root_file.Get("Energy")
            if not tree:
                print(f"Warning: Tree 'Energy' not found in {filename}. Skipping.")
                root_file.Close()
                continue
            energy_values = []
            for event in tree:
                energy_values.append(event.fEdep)
            # Apply Gaussian broadening if requested
            if apply_broadening:
                energy_values = apply_gaussian_broadening(energy_values)
            label = f"{dist} cm, {angle}°"
            color = colors[color_index]
            
            plt.hist(energy_values,
                    bins=650,
                    label=label,
                    histtype='stepfilled',
                    linewidth=0,
                    alpha=0.5,
                    color=color, 
                    density=True)
            plt.hist(energy_values,
                    bins=650,
                    histtype='step',
                    linewidth=2,
                    alpha=1.0,
                    color=color,
                    density=True)
            color_index += 1
            root_file.Close()
   
    plt.xlim(2250,3500)
    # Add plot details
    plt.xlabel('Energy Deposition [keV]')
    plt.ylabel('Simulated Distribution')
    
    if apply_broadening:
        title = 'Simulated Energy Deposition by 5486 keV Alphas in Silicon Diode in Air, With Gaussian Energy Broadening (16 keV FWHM @ 5486 keV)'
    else:
        title = 'Simulated Energy Deposition by 5486 keV Alphas in Silicon Diode in Air, No Gaussian Energy Broadening'
        
    plt.title(title)
    plt.legend(loc='upper right')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    # Save plot
    if output_filename:
        plt.savefig(output_filename, dpi=300)
    
    plt.close()

# Create both plots
#create_plot(apply_broadening=False, output_filename='energy_deposition_no_broadening.png')
create_plot(apply_broadening=True, output_filename='energy_deposition_with_broadening.png')


