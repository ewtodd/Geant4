import uproot
import matplotlib.pyplot as plt
import numpy as np
import mplhep as hep
import matplotlib as mpl
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
   # plt.title(title + " (Zoomed)")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + 'zoomed_.png')
    plt.close()

# Load the LaBr3 ROOT file
file_path_labr3 = "1E9_LaBr3_Corrected.root"
root_file_labr3 = uproot.open(file_path_labr3)
total_edep_labr3 = root_file_labr3["NaI"]["TotalEdep"].array(library="np")

# Plot for LaBr3 Total Edep
plot_histogram(total_edep_labr3, "Total Energy Deposition in LaBr3", "Energy Deposited (MeV)", "Counts", color="r")

# Apply Gaussian broadening for LaBr3
broadened_edep_labr3 = apply_energy_broadening(total_edep_labr3, labr3_resolution)
plot_histogram(broadened_edep_labr3, "Total Energy Deposition in LaBr3 (with broadening)", "Energy Deposited (MeV)", "Counts", color="r")

# Load the NaI ROOT file
file_path_nai = "1E9_NaI.root" 
root_file_nai = uproot.open(file_path_nai)
total_edep_nai = root_file_nai["NaI"]["TotalEdep"].array(library="np")

# Plot for NaI Total Edep
#plot_histogram(total_edep_nai, "Total Energy Deposition in NaI", "Energy Deposited (MeV)", "Counts", color="b")

# Apply Gaussian broadening for NaI
broadened_edep_nai = apply_energy_broadening(total_edep_nai, nai_resolution)
#plot_histogram(broadened_edep_nai, "Total Energy Deposition in NaI (with broadening)", "Energy Deposited (MeV)", "Counts", color="b") 

def gaussian_with_background(x, a, x0, sigma, m, b):
    """Gaussian with a linear background."""
    return a * np.exp(-(x - x0)**2 / (2 * sigma**2)) + m * x + b

def gaussian(x, a, x0, sigma):
    """Gaussian function only."""
    return a * np.exp(-(x - x0)**2 / (2 * sigma**2))

def linear_background(x, m, b):
    """Linear background function."""
    return m * x + b

def calculate_fwhm_energy_resolution(resolution_function, energy_mev):
    """Calculate FWHM based on the energy resolution function at a given energy."""
    relative_resolution = resolution_function(energy_mev)
    fwhm = relative_resolution * energy_mev
    return fwhm


def fit_peak(data, title, full_range=(1.8, 2.4), fit_range=(1.95, 2.12), bins=int(3000*(2.4-1.8)/8), resolution_function=None):
    # Create a histogram over the full range for plotting
    hist, bin_edges = np.histogram(data, bins=bins, range=full_range)
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2

    # Filter the data to include only within the fit range
    fit_mask = (bin_centers >= fit_range[0]) & (bin_centers <= fit_range[1])  # Fix: use [0] and [1] for range bounds
    fit_bin_centers = bin_centers[fit_mask]
    fit_hist = hist[fit_mask]

    # Calculate initial guess sigma using the expected FWHM from energy resolution
    if resolution_function:
        fwhm = calculate_fwhm_energy_resolution(resolution_function, 2.0)  # for 2 MeV
        sigma_guess = fwhm / (2 * np.sqrt(2 * np.log(2)))
    else:
        sigma_guess = 0.01  # Fallback default if no function is provided

    # Better initial parameter guesses
    peak_idx = np.argmax(fit_hist)
    peak_height = fit_hist[peak_idx] - np.min(fit_hist)
    background_level = np.min(fit_hist)
    p0 = [peak_height, fit_bin_centers[peak_idx], sigma_guess, 0, background_level]
    
    # Add bounds to prevent negative amplitude
    bounds = ([0, fit_range[0], sigma_guess/10, -np.inf, 0], 
          [np.inf, fit_range[1], sigma_guess*10, np.inf, np.inf])  # Fix: use [0] and [1] for range bounds
    popt, pcov = curve_fit(gaussian_with_background, fit_bin_centers, fit_hist, 
                       p0=p0, bounds=bounds)

    width=3

    # Create figure with two subplots: main plot and residuals
    fig, (ax1, ax2) = plt.subplots(2, 1, gridspec_kw={'height_ratios': [3, 1]}, sharex=True, figsize=(16, 12))

    # Plot the full data with the fit overlaid on the main plot
    ax1.plot(bin_centers, hist, label='Simulated Data', linewidth=width, color='black')

    # Generate a fit line over the full plot range using the fit parameters
    fit = gaussian_with_background(fit_bin_centers, *popt)
    ax1.plot(fit_bin_centers, fit, 'r--', label='Total Fit', linewidth=width)

    # Extract fitted parameters
    a, x0, sigma, m, b = popt

    # Plot Gaussian component separately using the fit parameters
    ax1.plot(fit_bin_centers, gaussian(fit_bin_centers, a, x0, sigma), 'g--', label='Photopeak', linewidth=width)

    # Plot Linear Background component separately
    ax1.plot(fit_bin_centers, linear_background(fit_bin_centers, m, b), 'b--', label='Background', linewidth=width)
    
    ax1.set_ylabel('Counts')
    #ax1.set_title(title + " Fit to 2 MeV Photopeak")
    ax1.legend()

    # Calculate residuals only for the fit range
    residuals = np.zeros_like(hist)
    residuals[fit_mask] = hist[fit_mask] - fit

    # Calculate sigma for residuals (standard deviation of residuals in fit range)
    residuals_std = np.std(residuals[fit_mask])

    # Calculate standardized residuals (residuals per sigma)
    standardized_residuals = np.zeros_like(hist)
    standardized_residuals[fit_mask] = residuals[fit_mask] / residuals_std

    # Plot standardized residuals
    ax2.plot(fit_bin_centers, standardized_residuals[fit_mask], 'k-', linewidth=width)
    ax2.axhline(0, color='red', linestyle='--')
    
       
    ax2.set_xlabel('Energy (MeV)')
    ax2.set_ylabel('Residuals / σ')
    
    # Set reasonable y-limits for the residual plot
    ax2.set_ylim(-3, 3)

    plt.tight_layout()
    plt.savefig(title+'fit_2MeV_peak_with_standardized_residuals.png')
    plt.close()

       # Calculate bin width
    bin_width = (full_range[1] - full_range[0]) / bins
    
    # Integrate the Gaussian and convert to counts
    gaussian_area, _ = quad(gaussian, fit_range[0], fit_range[1], args=(a, x0, sigma))
    peak_counts = gaussian_area / bin_width
    
    # Error propagation for peak counts
    def integral_gaussian(a, x0, sigma):
        area, _ = quad(gaussian, fit_range[0], fit_range[1], args=(a, x0, sigma))
        return area

    # Numerical partial derivatives
    def partial_a(a, x0, sigma):
        return integral_gaussian(a + 1e-6, x0, sigma) - integral_gaussian(a, x0, sigma)

    def partial_sigma(a, x0, sigma):
        return integral_gaussian(a, x0, sigma + 1e-6) - integral_gaussian(a, x0, sigma)

    # Calculate partial derivatives
    partial_a_val = partial_a(a, x0, sigma)
    partial_sigma_val = partial_sigma(a, x0, sigma)

    # Extract variances from covariance matrix
    var_a = pcov[0, 0]
    var_sigma = pcov[2, 2]

    # Error propagation formula
    peak_counts_error = np.sqrt((partial_a_val**2) * var_a + (partial_sigma_val**2) * var_sigma) / bin_width
    
    return peak_counts, peak_counts_error


# After broadening for LaBr3 and NaI:
labr3_counts_2mev, labr3_counts_2mev_error = fit_peak(broadened_edep_labr3, "LaBr3", resolution_function=labr3_resolution)
#nai_counts_2mev, nai_counts_2mev_error = fit_peak(broadened_edep_nai, "NaI", fit_range=(1.85, 2.23), resolution_function=nai_resolution)

# Extract number of 2 MeV gammas from "PreEnergy/GammaEnergies"
gamma_energies_labr3 = root_file_labr3["PreEnergy"]["GammaEnergies"].array(library="np")
gamma_counts_2mev_labr3 = np.sum(np.isclose(gamma_energies_labr3, 2.0, atol=0.05))

#gamma_energies_nai = root_file_nai["PreEnergy"]["GammaEnergies"].array(library="np")
#gamma_counts_2mev_nai = np.sum(np.isclose(gamma_energies_nai, 2.0, atol=0.05))

# Calculate efficiency with error propagation
efficiency_labr3 = 100 * labr3_counts_2mev / gamma_counts_2mev_labr3
efficiency_labr3_error = 100 * labr3_counts_2mev_error / gamma_counts_2mev_labr3

#efficiency_nai = 100 * nai_counts_2mev / gamma_counts_2mev_nai
#efficiency_nai_error = 100 * nai_counts_2mev_error / gamma_counts_2mev_nai

# Print or log the comparison with errors
print(f"LaBr3: Fitted counts = {labr3_counts_2mev:.2f} ± {labr3_counts_2mev_error:.2f}, 2 MeV gammas = {gamma_counts_2mev_labr3}, Photopeak Efficiency (%) = {efficiency_labr3:.4f} ± {efficiency_labr3_error:.4f}")
#print(f"NaI: Fitted counts = {nai_counts_2mev:.2f} ± {nai_counts_2mev_error:.2f}, 2 MeV gammas = {gamma_counts_2mev_nai}, Photopeak Efficiency (%) = {efficiency_nai:.4f} ± {efficiency_nai_error:.4f}")

