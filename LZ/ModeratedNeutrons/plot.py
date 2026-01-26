import uproot
import matplotlib.pyplot as plt
import numpy as np
import mplhep as hep
import matplotlib as mpl
from matplotlib.colors import LogNorm
from scipy.optimize import curve_fit
from scipy.integrate import quad
mpl.use('Agg')
hep.style.use(hep.style.ROOT)
plt.rcParams['figure.figsize'] = [16, 10]
plt.rcParams['legend.facecolor'] = 'white'
plt.rcParams['legend.framealpha'] = 1
plt.rcParams['legend.edgecolor'] = 'black'
plt.rcParams['legend.fancybox'] = True
plt.rcParams['legend.frameon'] = True
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Liberation Serif']
def apply_energy_broadening(energies, resolution_function):
    mask = energies > 0
    result = np.copy(energies)
    if np.any(mask):
        valid_energies = energies[mask]
        relative_resolution = resolution_function(valid_energies)
        fwhm = relative_resolution * valid_energies
        sigma = fwhm / (2 * np.sqrt(2 * np.log(2)))
        result[mask] = np.random.normal(valid_energies, sigma)
    return result
def labr3_resolution(energy_mev):
    """LaBr3(Ce) 1.5"×1.5" resolution function
    Calibrated to match multiple data points:
    - 2.9% at 662 keV
    - 2.1% at 1332 keV
    - 1.6% at 2615 keV
    """
    # Parameters from curve fitting (search result [1])
    a = 0.00671  # constant term
    b = 0.00895  # statistical term
    c = 0.00000213  # quadratic term (very small)
    # Convert to FWHM
    fwhm_factor = 2 * np.sqrt(2 * np.log(2))
    sigma = np.sqrt(a**2 + (b**2)/energy_mev + (c**2)/(energy_mev**2))
    return fwhm_factor * sigma  # Return FWHM/E (relative resolution)
def nai_resolution(energy_mev):
    """NaI(Tl) 2"×2" resolution function
    Calibrated to match:
    - 7.0% at 662 keV
    - 5.4% at 1332 keV
    - 4.5% at 2615 keV
    """
    # Parameters fitted to match all three points
    a = 0.026  # constant term
    b = 0.058  # statistical term
    c = 0.0    # no significant quadratic term needed
    return np.sqrt(a**2 + (b**2)/energy_mev + (c**2)/(energy_mev**2))

def plot_histogram(data, title, xlabel, ylabel, color="b"):
    hist = np.histogram(data, bins=3000, range=(5e-2, 8))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    #plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + '.png')
    plt.close()
    hist = np.histogram(data, bins=int((2/8)*3000), range=(1, 3))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    #plt.title(title + " (Zoomed)")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + 'zoomed_.png')
    plt.close()
# Load the LaBr3 ROOT file
file_path_labr3 = "leadwall1E7.root"
root_file_labr3 = uproot.open(file_path_labr3)
total_edep_labr3 = root_file_labr3["LaBr3"]["fEDep"].array(library="np")
# Plot for LaBr3 Total Edep
plot_histogram(total_edep_labr3, "Total Energy Deposition in LaBr3", "Energy Deposited [MeV]", "Counts", color="r")
# Apply Gaussian broadening for LaBr3
broadened_edep_labr3 = apply_energy_broadening(total_edep_labr3, labr3_resolution)
plot_histogram(broadened_edep_labr3, "Total Energy Deposition in LaBr3 (with broadening)", "Energy Deposited [MeV]", "Counts", color="r")

def plot_histogram(data, title, xlabel, ylabel, color="b"):
    hist = np.histogram(data, bins=3000, range=(5e-2, 3))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    #plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + 'coincidence_data.png')
    plt.close()
# Add this new plotting function
def plot_coincidence_hist(data1, data2, times1, times2, title, xlabel, ylabel):
    # Calculate time differences (assuming times are in ns)
    time_diff = times1 - times2
    # Create mask for 50 ns coincidence window
    mask = np.abs(time_diff) <= 0.1
    # Create 2D histogram of coincident events
    hist = np.histogram2d(
        data1[mask],
        data2[mask],
        bins=(100,100), range=((5e-2,2.5),(5e-2,2.5))
    )
    plt.figure()
            
    h = hep.hist2dplot(hist, norm=LogNorm(), cbar=True)
    cbar = h[1]  # mplhep returns a dict with the colorbar under 'cbar'
    cbar.set_label('Counts')  # Set your desired label here
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + '.png')
    plt.close()
# Modify your data loading section to include times
def load_detector_data(file_path, detector_name):
    root_file = uproot.open(file_path)
    return {
        "energy": root_file[detector_name]["fEDep"].array(library="np"),
        "time": root_file[detector_name]["fTime"].array(library="np")
    }
# Load data for both detectors
labr3_data = load_detector_data("sumcoincidence.root", "LaBr3")
cebr3_data = load_detector_data("sumcoincidence.root", "CeBr3")  # Update with your CeBr3 file
# Plot for LaBr3 Total Edep
plot_histogram(total_edep_labr3, "Total Energy Deposition in LaBr3", "Energy Deposited [MeV]", "Counts", color="r")
broadened_edep_labr3 = apply_energy_broadening(labr3_data["energy"], labr3_resolution)
plot_histogram(broadened_edep_labr3, "Total Energy Deposition in LaBr3 (with broadening)", "Energy Deposited [MeV]", "Counts", color="r")
broadened_edep_cebr3 = apply_energy_broadening(cebr3_data["energy"], nai_resolution)
plot_histogram(broadened_edep_cebr3, "Total Energy Deposition in CeBr3 (with broadening)", "Energy Deposited [MeV]", "Counts", color="r")
labr3_data["energy"] = broadened_edep_labr3
cebr3_data["energy"] = broadened_edep_cebr3
# Create coincidence plot
plot_coincidence_hist(
    labr3_data["energy"],
    cebr3_data["energy"],
    labr3_data["time"],
    cebr3_data["time"],
    "LaBr3_vs_CeBr3_Coincidence",
    "LaBr3 Energy (MeV)",
    "CeBr3 Energy (MeV)"
)
def plot_histogram_time(data, title, xlabel, ylabel, color="b"):
    hist = np.histogram(data, bins=3000, range=(5e-2, np.max(data)))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.title(title)
    plt.yscale('log')
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + 'coincidence_data.png')
    plt.close()
plot_histogram_time(labr3_data["time"],"LaBr3 Time Distribution", "Time [ns]", "Counts")
plot_histogram_time(cebr3_data["time"],"CeBr3 Time Distribution", "Time [ns]", "Counts")
