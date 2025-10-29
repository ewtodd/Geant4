import uproot
import matplotlib.pyplot as plt
import numpy as np
import mplhep as hep
import matplotlib as mpl
from matplotlib.colors import LogNorm
from scipy.optimize import curve_fit
from scipy.integrate import quad

mpl.use("Agg")
hep.style.use(hep.style.ROOT)
plt.rcParams["figure.figsize"] = [16, 10]
plt.rcParams["legend.facecolor"] = "white"
plt.rcParams["legend.framealpha"] = 1
plt.rcParams["legend.edgecolor"] = "black"
plt.rcParams["legend.fancybox"] = True
plt.rcParams["legend.frameon"] = True
plt.rcParams["font.family"] = "serif"
plt.rcParams["font.serif"] = ["Liberation Serif"]


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


def nai_3x3_resolution(energy_kev):
    """NaI(Tl) 3"×3" resolution function
    Calibrated to match:
    - 7.5% at 662 keV (typical for 3x3 inch)
    - Scaled from 2x2 inch performance
    """
    # Convert keV to MeV for calculation
    energy_mev = energy_kev / 1000.0
    # Parameters fitted for 3x3 inch crystal
    a = 0.030  # constant term (slightly higher than 2x2)
    b = 0.062  # statistical term
    c = 0.0  # no significant quadratic term needed
    return np.sqrt(a**2 + (b**2) / energy_mev + (c**2) / (energy_mev**2))


def cdte_resolution(energy_kev):
    """CdTe detector resolution function
    Calibrated to match Amptek 25mm² performance:
    - ~1.0 keV FWHM at 122 keV (0.82%)
    - ~1.5 keV FWHM at 122 keV (1.23%) for 25mm²
    Extrapolated to higher energies considering hole trapping
    """
    # Convert keV to MeV for calculation
    energy_mev = energy_kev / 1000.0
    # Parameters for CdTe detector (1mm thick)
    a = 0.008  # constant term (better than NaI)
    b = 0.012  # statistical term (much lower than NaI)
    c = 0.0015  # small quadratic term for hole trapping effects
    return np.sqrt(a**2 + (b**2) / energy_mev + (c**2) / (energy_mev**2))


def plot_histogram_cdte(data, title, xlabel, ylabel, color="b"):
    # Define bins for CdTe: 0 to 100 keV in 10 keV bins
    bin_edges = np.arange(1e-2, 110, 1)  # 0 to 100 keV in 10 keV bins
    counts, edges = np.histogram(data, bins=bin_edges)

    # Normalize to counts per 10 keV (bin width is already 10 keV)
    bin_width = edges[1] - edges[0]
    counts_per_10kev = counts / (bin_width / 10.0)

    hist = (counts_per_10kev, edges)
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.yscale("log")
    plt.savefig(title + ".png")
    plt.close()


def plot_histogram_nai(data, title, xlabel, ylabel, color="b"):
    # Define bins for NaI: 0 to 6000 keV (6 MeV) in 10 keV bins
    bin_edges = np.arange(5600, 6000, 5)  # 0 to 6000 keV in 10 keV bins
    counts, edges = np.histogram(data, bins=bin_edges)

    # Normalize to counts per 10 keV
    bin_width = edges[1] - edges[0]
    counts_per_10kev = counts / (bin_width / 10.0)

    hist = (counts_per_10kev, edges)
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.yscale("log")
    plt.savefig(title + ".png")
    plt.close()


# Add this new plotting function for coincidence
def plot_coincidence_hist(data1, data2, times1, times2, title, xlabel, ylabel):
    # Calculate time differences (assuming times are in ns)
    time_diff = times1 - times2
    # Create mask for 0.1 ns coincidence window
    mask = np.abs(time_diff) <= 0.1

    # Create 2D histogram of coincident events with 10 keV bins
    # CdTe: 0-100 keV, NaI: 0-6000 keV
    bin_edges_cdte = np.arange(1e-2, 110, 1)  # 10 keV bins for CdTe
    bin_edges_nai = np.arange(5600, 6000, 5)  # 10 keV bins for NaI

    counts, xedges, yedges = np.histogram2d(
        data1[mask], data2[mask], bins=(bin_edges_cdte, bin_edges_nai)
    )

    # Normalize to counts per 10 keV² (per bin area)
    bin_width_x = xedges[1] - xedges[0]
    bin_width_y = yedges[1] - yedges[0]
    counts_normalized = counts / ((bin_width_x / 10.0) * (bin_width_y / 10.0))

    hist = (counts_normalized, xedges, yedges)
    plt.figure()

    h = hep.hist2dplot(hist, norm=LogNorm(), cbar=True)
    cbar = h[1]  # mplhep returns a dict with the colorbar under 'cbar'
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + ".png")
    plt.close()


# Modify your data loading section to include times
def load_detector_data(file_path, detector_name):
    root_file = uproot.open(file_path)
    return {
        "energy": root_file[detector_name]["fEDep"].array(library="np")
        * 1000.0,  # Convert to keV
        "time": root_file[detector_name]["fTime"].array(library="np"),
    }


# Load data for both detectors
ge_data = load_detector_data("1E8.root", "Ge")
cdte_data = load_detector_data("1E8.root", "CdTe")
nai_data = load_detector_data("1E8.root", "NaI")

# Plot for CdTe Total Edep (0-100 keV)
total_edep_ge = ge_data["energy"]
plot_histogram_cdte(
    total_edep_ge,
    "Total Energy Deposition in Ge",
    "Energy Deposited [keV]",
    "Counts / keV",
    color="g",
)

broadened_edep_cdte = apply_energy_broadening(cdte_data["energy"], cdte_resolution)
plot_histogram_cdte(
    broadened_edep_cdte,
    "Total Energy Deposition in CdTe (with broadening)_coincidence",
    "Energy Deposited [keV]",
    "Counts / keV",
    color="r",
)

# Plot for NaI Total Edep (0-6 MeV)
broadened_edep_nai = apply_energy_broadening(nai_data["energy"], nai_3x3_resolution)
plot_histogram_nai(
    broadened_edep_nai,
    "Total Energy Deposition in NaI (with broadening)",
    "Energy Deposited [keV]",
    "Counts / 5 keV",
    color="b",
)

cdte_data["energy"] = broadened_edep_cdte
nai_data["energy"] = broadened_edep_nai

# Create coincidence plot (CdTe: 0-100 keV, NaI: 0-6000 keV)
plot_coincidence_hist(
    cdte_data["energy"],
    nai_data["energy"],
    cdte_data["time"],
    nai_data["time"],
    "CdTe_vs_NaI_Coincidence",
    "CdTe Deposited Energy [keV]",
    "NaI Deposited Energy [keV]",
)
