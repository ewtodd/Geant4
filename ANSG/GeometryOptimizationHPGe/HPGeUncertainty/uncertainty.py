import numpy as np
import matplotlib.pyplot as plt

def calculate_quenching_factor(E_gamma_nr, sigma_E_gamma_nr, E_nr, sigma_E_nr, E_gamma, sigma_E_gamma):
    # Calculate ionization energy
    E_ion = E_gamma_nr - E_gamma
    if E_ion <= 0:
        return 0, 0, 0 
    # Calculate uncertainties
    sigma_E_ion_squared = sigma_E_gamma_nr**2 + sigma_E_gamma**2
    
    # Calculate quenching factor
    QF = (E_ion / E_nr) * 100  # Convert to percentage
    
    # Calculate uncertainty in quenching factor
    sigma_QF_squared = ((E_ion / E_nr)**2) * ((sigma_E_ion_squared / E_ion**2) + (sigma_E_nr**2 / E_nr**2))
    sigma_QF = np.sqrt(sigma_QF_squared) * 100  # Convert to percentage
    
    # Calculate contribution of E_gamma uncertainty to total uncertainty
    contribution_E_gamma = (sigma_E_gamma**2) / sigma_E_ion_squared
    
    return QF, sigma_QF, contribution_E_gamma

# Parameters
sigma_E_gamma_nr = 0.0046
E_nr = 254.1 / 1000
sigma_E_nr = 1.5 / 1000

# Create ranges for E_gamma_nr, E_gamma, and sigma_E_gamma
E_gamma_nr_range = np.linspace(68.7, 68.822, 200)
E_gamma_range = np.linspace(68.650, 68.8, 200)
sigma_E_gamma_range = 0.007

# Calculate quenching factor for each combination of E_gamma_nr and E_gamma
QF_2D = np.zeros((len(E_gamma_nr_range), len(E_gamma_range)))

for i, E_gamma_nr in enumerate(E_gamma_nr_range):
    for j, E_gamma in enumerate(E_gamma_range):
        QF, sigma_QF, contribution = calculate_quenching_factor(
            E_gamma_nr, sigma_E_gamma_nr, E_nr, sigma_E_nr, E_gamma, sigma_E_gamma_range
        )
        QF_2D[i, j] = QF

# Create the quenching factor plot with Lindhard contour
fig, ax1 = plt.subplots(figsize=(8, 7))


# Quenching Factor plot
im1 = ax1.imshow(QF_2D, extent=[E_gamma_range[0], E_gamma_range[-1], 
                                E_gamma_nr_range[0], E_gamma_nr_range[-1]], 
                 aspect='auto', origin='lower', cmap='plasma')  # Changed colormap to 'plasma'

ax1.set_xlabel(r'$E_{\gamma}$ [keV]')
ax1.set_ylabel(r'$E_{\gamma\text{nr}}$ [keV]')
plt.colorbar(im1, ax=ax1, label='Ionization Yield (%)')

# Add contour line for quenching factor equal to 14.3%
CS_QF = ax1.contour(E_gamma_range, E_gamma_nr_range, QF_2D, levels=[14.3], colors='white',linewidths=2)
ax1.clabel(CS_QF, inline=True, fontsize=10, fmt='%1.1f%%')
# Mark the special point
UM_E_gamma_nr = 68.816
UM_E_gamma = 68.752
UM_gamma_err = 0.007
UM_gamma_nr_err = 0.005

UC_E_gamma_nr = 68.811
UC_E_gamma = 68.734
UC_gamma_err = 0.02
UC_gamma_nr_err = 0.001


B_E_gamma_nr = 68.793
B_E_gamma = 68.754 
B_gamma_err = 0.0043
B_gamma_nr_err = 0.0034

# UMich 2024 point - use a soft lavender
ax1.errorbar(UM_E_gamma, UM_E_gamma_nr, 
             xerr=UM_gamma_err, yerr=UM_gamma_nr_err,
             marker='.', color='pink', markersize=10, 
             label='UMich 2024')

# UChicago 2021 point - use a light coral
ax1.errorbar(UC_E_gamma, UC_E_gamma_nr,
             xerr=UC_gamma_err, yerr=UC_gamma_nr_err,
             marker='.', color='purple', markersize=10,
             label='UChigago 2021')

# Brookhaven 1975 point - use a pale turquoise
ax1.errorbar(B_E_gamma, B_E_gamma_nr,
             xerr=B_gamma_err, yerr=B_gamma_nr_err,
             marker='.', color='yellow', markersize=10,
             label='Brookhaven 1975')
ax1.legend(loc=4)
plt.tight_layout()
plt.savefig('quenching_factor_Egamma_Egammanr.png')
plt.close()

# Create the contribution plot for a specific E_gamma_nr value
E_gamma_nr = 68.8158
# Create ranges for E_gamma and sigma_E_gamma
E_gamma_range = np.linspace(68.734, 68.81, 100)
sigma_E_gamma_range = np.linspace(0.0005, 0.010, 100)
QF_2D = np.zeros((len(sigma_E_gamma_range), len(E_gamma_range)))
sigma_QF_2D = np.zeros_like(QF_2D)
contribution_2D = np.zeros_like(QF_2D)

for i, E_gamma in enumerate(E_gamma_range):
    for j, sigma_E_gamma in enumerate(sigma_E_gamma_range):
        QF, sigma_QF, contribution = calculate_quenching_factor(
            E_gamma_nr, sigma_E_gamma_nr, E_nr, sigma_E_nr, E_gamma, sigma_E_gamma
        )
        QF_2D[j, i] = QF
        sigma_QF_2D[j, i] = sigma_QF
        contribution_2D[j, i] = contribution * 100

# Contribution plot

# Contribution plot
fig, ax2 = plt.subplots(figsize=(8, 7))

im2 = ax2.imshow(contribution_2D, extent=[E_gamma_range[0], E_gamma_range[-1], 
                                          sigma_E_gamma_range[0], sigma_E_gamma_range[-1]], 
                 aspect='auto', origin='lower', cmap='plasma')  # Changed colormap to 'plasma'
ax2.set_xlabel(r'$E_{\gamma}$ [keV]')
ax2.set_ylabel(r'$\sigma_{E_{\gamma}}$ [keV]')
ax2.set_title(r'Contribution of $E_{\gamma}$ to total uncertainty')
plt.colorbar(im2, ax=ax2, label='Contribution (%)')

# Add contour lines for 50% contribution
CS = ax2.contour(E_gamma_range, sigma_E_gamma_range, contribution_2D, levels=[50], colors='white', linewidths=2)
ax2.clabel(CS, inline=True, fontsize=10, fmt='%1.0f%%')
# Mark the point from the paper
ax2.plot(68.752, 0.007, 'rx', markersize=10, label='IAEA Database')
ax2.legend()

plt.tight_layout()
plt.savefig('contribution_plot.png')
plt.close()
