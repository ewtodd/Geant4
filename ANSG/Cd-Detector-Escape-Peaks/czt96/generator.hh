#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4ParticleGun.hh"
#include "G4ThreeVector.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

// The 95.902 keV E2 transition of 114Cd after 113Cd(n,g) captures inside the
// CZT. ESC96_MODE selects where the captures happen:
//   surface (default): thermal neutrons entering every face, weighted by area,
//     with a cosine-law current, captured after an exponential path of mean
//     ESC96_MFP_MM (default 0.25 mm);
//   uniform: anywhere in the crystal volume;
//   beam: an external pencil beam of photons straight down onto crystal 3.
// Unless ESC96_CONVERSION=0, the transition converts as BrIccFO gives for E2
// (alpha_K 1.288, alpha_T 1.735): K conversion emits the electron and then a
// Cd K X-ray (omega_K 0.842, DDEP line intensities) plus the rest of the K
// binding energy as a local electron, or an Auger electron; L and higher
// conversion emits the electron and the binding energy as a local electron.
class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  ~PrimaryGenerator();

  virtual void GeneratePrimaries(G4Event *);

private:
  G4ThreeVector SurfaceCapturePoint() const;
  G4ThreeVector UniformPoint() const;
  void Fire(G4Event *anEvent, G4ParticleDefinition *particle, G4double energy,
            const G4ThreeVector &position);

  G4ParticleGun *fGun;
  G4String fMode;
  G4double fMfp;
  G4bool fConversion;
};

#endif
