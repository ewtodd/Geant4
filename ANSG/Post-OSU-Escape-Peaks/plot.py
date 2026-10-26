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
plt.rcParams["figure.figsize"] = [15, 12]
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


def plot_histogram_cdte(data, title, xlabel, ylabel, color="b", bins=600):

    data = data[(data > 50) & (data < 110)]
    hist = np.histogram(data, bins=bins)

    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.vlines(
        95.902,
        0,
        2e5,
        linewidth=3.0,
        color="b",
        linestyle="dashed",
        label="95.902 keV, 113Cd(n,g)",
    )
    plt.vlines(
        95.902 - 26.7112,
        0,
        2e5,
        linewidth=3.0,
        linestyle="dashed",
        color="g",
        label="95.902 keV - Cd K-edge",
    )
    plt.vlines(
        95.902 - 23.1739,
        0,
        2e5,
        linewidth=3.0,
        linestyle="dashed",
        color="r",
        label="95.902 keV - Cd K-alpha",
    )
    plt.vlines(
        75.177,
        0,
        2e5,
        linewidth=3.0,
        color="grey",
        linestyle="dashed",
        label="75.177 keV, 113Cd(n,g)",
    )

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.yscale("log")
    plt.legend(loc="upper left", bbox_to_anchor=(1, 1))

    plt.savefig(title + ".png", bbox_inches="tight")
    plt.close()


# Modify your data loading section to include times
def load_detector_data(file_path, detector_name):
    root_file = uproot.open(file_path)
    return {
        "energy": root_file[detector_name]["fEDep"].array(library="np")
        * 1000.0,  # Convert to keV
    }


cdte_data = load_detector_data("1E7.root", "CdTe")


broadened_edep_cdte = apply_energy_broadening(cdte_data["energy"], cdte_resolution)
plot_histogram_cdte(
    cdte_data["energy"],
    "Total Energy Deposition in CdTe",
    "Energy Deposited [keV]",
    "Counts",
    color="r",
    bins=10000,
)

plot_histogram_cdte(
    broadened_edep_cdte,
    "Total Energy Deposition in CdTe (with broadening)",
    "Energy Deposited [keV]",
    "Counts",
    color="r",
)

cdte_data["energy"] = broadened_edep_cdte
