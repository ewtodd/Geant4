import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import mplhep as hep

mpl.use('Agg')
hep.style.use(hep.style.ROOT)
plt.rcParams['figure.figsize'] = [12, 10]
plt.rcParams['legend.facecolor'] = 'white'    # Legend background color
plt.rcParams['legend.framealpha'] = 1         # Legend background opacity (1 = opaque, 0 = transparent)
plt.rcParams['legend.edgecolor'] = 'black'    # Legend border color
plt.rcParams['legend.fancybox'] = True        # Rounded box corners (optional, default True)
plt.rcParams['legend.frameon'] = True         # Show legend frame (default True)
plt.rcParams['font.family'] = 'serif'
plt.rcParams['font.serif'] = ['Liberation Serif']

def ansg(text):
    hep.label.exp_text(
    exp="ANSG",  
    text=text,   
    loc=0,                
    italic=(False,True,False),
    fontname=plt.rcParams['font.serif'][0]
    )


# Watt spectrum parameters for Cf-252
a = 1.025  # MeV
b = 2.926  # MeV^-1

# Define the Watt spectrum function
def watt_spectrum(E):
    return np.exp(-E/a) * np.sinh(np.sqrt(b*E))

# Energy range from 0.001 to 100 MeV (log scale)
E = np.logspace(-5, 2, 250)

# Calculate spectrum values
P = watt_spectrum(E)

# Normalize for better visualization
P = P / np.max(P)

fig, ax = plt.subplots()
plt.plot(E, P, '-', linewidth=3, label='Cf-252 Watt Spectrum', color='red')
plt.fill_between(E,P,color='red', alpha=0.5)
plt.xscale('log')
plt.xlim(0.00001, 100)
plt.xlabel('Neutron Energy [MeV]')
plt.ylabel('Normalized Probability')
plt.legend()
ansg("Simulation")
plt.grid()
plt.tight_layout()
plt.savefig('prob.png')

