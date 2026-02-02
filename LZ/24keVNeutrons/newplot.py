
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

def plot_histogram(data, title, xlabel, ylabel, color="b"):
    hist = np.histogram(data, bins=3000, range=(5e-2, 8))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + '.png')
    plt.close()
    hist = np.histogram(data, bins=int((2/8)*3000), range=(1, 3))
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + 'zoomed_.png')
    plt.close()

def plot_histogram_range(data, title, xlabel, ylabel, color="b", range=(5e-2, 3)):
    hist = np.histogram(data, bins=3000, range=range)
    hep.histplot(hist, histtype="fill", alpha=0.5, color=color)
    hep.histplot(hist, histtype="step", color=color)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.savefig(title + '.png')
    plt.close()

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

def find_anti_coincidence_mask(times1, times2, window=0.1):
    # Sort times2 for efficient search
    times2_sorted = np.sort(times2)
    mask = np.ones_like(times1, dtype=bool)
    
    # Process in batches to show progress
    batch_size = 10000
    total_batches = len(times1) // batch_size + (1 if len(times1) % batch_size > 0 else 0)
    
    for batch in range(total_batches):
        start_idx = batch * batch_size
        end_idx = min((batch + 1) * batch_size, len(times1))
        
        if batch % 10 == 0:
            print(f"Processing batch {batch+1}/{total_batches}")
        
        for i in range(start_idx, end_idx):
            t1 = times1[i]
            # Use np.searchsorted to find closest event in times2
            idx = np.searchsorted(times2_sorted, t1)
            # Check neighbors for coincidence
            coincident = False
            for j in [idx-1, idx]:
                if 0 <= j < len(times2_sorted):
                    if abs(t1 - times2_sorted[j]) <= window:
                        coincident = True
                        break
            mask[i] = not coincident
    
    return mask

def plot_coincidence_hist(data1, data2, times1, times2, title, xlabel, ylabel):
    # Find coincident events using a more memory-efficient approach
    times1_array = np.array(times1)
    times2_array = np.array(times2)
    
    # Sort both arrays for efficient searching
    sort_idx1 = np.argsort(times1_array)
    sort_idx2 = np.argsort(times2_array)
    
    sorted_times1 = times1_array[sort_idx1]
    sorted_times2 = times2_array[sort_idx2]
    
    # Find coincidences within window
    coincidence_pairs = []
    window = 0.1  # 0.1 ns coincidence window
    
    # Limit to a reasonable number of points for the plot
    max_points = 10000
    
    for i, t1 in enumerate(sorted_times1[:max_points]):
        idx = np.searchsorted(sorted_times2, t1)
        for j in [idx-1, idx]:
            if 0 <= j < len(sorted_times2):
                if abs(t1 - sorted_times2[j]) <= window:
                    coincidence_pairs.append((sort_idx1[i], sort_idx2[j]))
    
    if coincidence_pairs:
        # Extract coincident event energies
        coincident_idx1, coincident_idx2 = zip(*coincidence_pairs)
        coincident_energies1 = data1[list(coincident_idx1)]
        coincident_energies2 = data2[list(coincident_idx2)]
        
        # Create 2D histogram
        hist = np.histogram2d(
            coincident_energies1,
            coincident_energies2,
            bins=(100, 100), range=((5e-2, 2.5), (5e-2, 2.5))
        )
        
        plt.figure()
        h = hep.hist2dplot(hist, norm=LogNorm(), cbar=True)
        cbar = h[1]
        cbar.set_label('Counts')
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.grid(True)
        plt.savefig(title + '.png')
        plt.close()
    else:
        print(f"No coincident events found for {title}")

# Load detector data function
def load_detector_data(file_path, detector_name):
    root_file = uproot.open(file_path)
    return {
        "energy": root_file[detector_name]["fEDep"].array(library="np"),
        "time": root_file[detector_name]["fTime"].array(library="np")
    }

# Load data for both detectors
print("Loading data...")
labr3_data = load_detector_data("antitest.root", "LaBr3")
cebr3_data = load_detector_data("antitest.root", "CeBr3")
print(f"Loaded {len(labr3_data['energy'])} LaBr3 events and {len(cebr3_data['energy'])} CeBr3 events")

# Apply energy broadening
print("Applying energy broadening...")
broadened_edep_labr3 = apply_energy_broadening(labr3_data["energy"], labr3_resolution)
broadened_edep_cebr3 = apply_energy_broadening(cebr3_data["energy"], labr3_resolution)

# Plot original LaBr3 spectrum with broadening
print("Plotting original LaBr3 spectrum...")
plot_histogram_range(broadened_edep_labr3, 
                    "Total_Energy_Deposition_in_LaBr3_with_broadening", 
                    "Energy Deposited [MeV]", 
                    "Counts", 
                    color="r")

# Create anti-coincidence mask using memory-efficient approach
print("Finding anti-coincidence events...")
anti_coincidence_mask = find_anti_coincidence_mask(
    labr3_data["time"], 
    cebr3_data["time"], 
    window=0.1
)

# Apply anti-coincidence mask to get LaBr3 events without CeBr3 coincidence
labr3_anti_coincidence = broadened_edep_labr3[anti_coincidence_mask]
print(f"Found {len(labr3_anti_coincidence)} anti-coincidence events out of {len(broadened_edep_labr3)} total events")

# Plot LaBr3 spectrum after removing coincidence events
print("Plotting anti-coincidence spectrum...")
plot_histogram_range(labr3_anti_coincidence, 
                    "LaBr3_Energy_Spectrum_After_Removing_Coincidence", 
                    "Energy Deposited [MeV]", 
                    "Counts", 
                    color="b")

# Plot both spectra on the same figure for comparison
print("Creating comparison plots...")
plt.figure(figsize=(16, 10))
# Original spectrum
hist_original = np.histogram(broadened_edep_labr3, bins=3000, range=(5e-2, 3))
hep.histplot(hist_original, histtype="step", color="r", label="Original LaBr3 Spectrum")

# Anti-coincidence spectrum
hist_anti = np.histogram(labr3_anti_coincidence, bins=3000, range=(5e-2, 3))
hep.histplot(hist_anti, histtype="step", color="b", label="LaBr3 After Anti-Coincidence")

plt.xlabel("Energy Deposited [MeV]")
plt.ylabel("Counts")
plt.grid(True)
plt.legend()
plt.savefig("LaBr3_Spectrum_Comparison.png")
plt.close()

# Create a zoomed version of the comparison plot for the region of interest
plt.figure(figsize=(16, 10))
# Original spectrum zoomed
hist_original_zoom = np.histogram(broadened_edep_labr3, bins=int((2/8)*3000), range=(1, 3))
hep.histplot(hist_original_zoom, histtype="step", color="r", label="Original LaBr3 Spectrum")

# Anti-coincidence spectrum zoomed
hist_anti_zoom = np.histogram(labr3_anti_coincidence, bins=int((2/8)*3000), range=(1, 3))
hep.histplot(hist_anti_zoom, histtype="step", color="b", label="LaBr3 After Anti-Coincidence")

plt.xlabel("Energy Deposited [MeV]")
plt.ylabel("Counts")
plt.grid(True)
plt.legend()
plt.savefig("LaBr3_Spectrum_Comparison_zoomed.png")
plt.close()

# Plot time distributions
print("Plotting time distributions...")
plot_histogram_time(labr3_data["time"], "LaBr3_Time_Distribution", "Time [ns]", "Counts")
plot_histogram_time(cebr3_data["time"], "CeBr3_Time_Distribution", "Time [ns]", "Counts")

# Plot coincidence histogram with memory-efficient approach
print("Creating coincidence plot...")
plot_coincidence_hist(
    broadened_edep_labr3,
    broadened_edep_cebr3,
    labr3_data["time"],
    cebr3_data["time"],
    "LaBr3_vs_CeBr3_Coincidence",
    "LaBr3 Energy (MeV)",
    "CeBr3 Energy (MeV)"
)

print("All plots completed!")

