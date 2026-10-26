#include "generator.hh"

#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cstdlib>

namespace {
// Must match construction.cc.
const G4double kHalf[3] = {10.375 * mm, 10.375 * mm, 5.0 * mm};
const G4double kCenter = 11.256 * mm;

const G4double kLineEnergy = 95.902 * keV;
const G4double kAlphaK = 1.288, kAlphaTotal = 1.735;
const G4double kOmegaK = 0.842;
const G4double kCdKBinding = 26.711 * keV;
const G4double kCdLBinding = 3.8 * keV;
const G4int kNCdLines = 4;
const G4double kCdLineEnergy[kNCdLines] = {23.1739 * keV, 22.9843 * keV,
                                           26.095 * keV, 26.644 * keV};
const G4double kCdLineIntensity[kNCdLines] = {1.0, 0.5317, 0.279, 0.051};

G4ThreeVector CrystalCenter(G4int copy) {
  G4double x = (copy % 2 == 0) ? -kCenter : kCenter;
  G4double y = (copy < 2) ? -kCenter : kCenter;
  return G4ThreeVector(x, y, 0.0);
}

G4double PickCdXray() {
  G4double total = 0;
  for (G4int k = 0; k < kNCdLines; k++)
    total += kCdLineIntensity[k];
  G4double pick = G4UniformRand() * total;
  for (G4int k = 0; k < kNCdLines; k++) {
    if (pick < kCdLineIntensity[k])
      return kCdLineEnergy[k];
    pick -= kCdLineIntensity[k];
  }
  return kCdLineEnergy[kNCdLines - 1];
}
} // namespace

PrimaryGenerator::PrimaryGenerator() {
  fGun = new G4ParticleGun(1);
  const char *mode = std::getenv("ESC96_MODE");
  fMode = mode ? G4String(mode) : G4String("surface");
  const char *mfp = std::getenv("ESC96_MFP_MM");
  fMfp = (mfp ? std::atof(mfp) : 0.25) * mm;
  const char *conversion = std::getenv("ESC96_CONVERSION");
  fConversion = !(conversion && std::atoi(conversion) == 0);
}

PrimaryGenerator::~PrimaryGenerator() { delete fGun; }

G4ThreeVector PrimaryGenerator::SurfaceCapturePoint() const {
  G4double area[3] = {kHalf[1] * kHalf[2], kHalf[0] * kHalf[2],
                      kHalf[0] * kHalf[1]};
  G4double total = area[0] + area[1] + area[2];
  G4double pick = G4UniformRand() * total;
  G4int axis = (pick < area[0]) ? 0 : (pick < area[0] + area[1]) ? 1 : 2;
  G4double side = (G4UniformRand() < 0.5) ? -1.0 : 1.0;

  G4double depth = 0;
  do {
    G4double cos_theta = std::sqrt(G4UniformRand());
    depth = -fMfp * std::log(G4UniformRand()) * cos_theta;
  } while (depth >= 2.0 * kHalf[axis]);

  G4double local[3];
  for (G4int k = 0; k < 3; k++)
    local[k] = (2.0 * G4UniformRand() - 1.0) * kHalf[k];
  local[axis] = side * (kHalf[axis] - depth);

  G4int copy = (G4int)(4.0 * G4UniformRand());
  return CrystalCenter(copy) + G4ThreeVector(local[0], local[1], local[2]);
}

G4ThreeVector PrimaryGenerator::UniformPoint() const {
  G4int copy = (G4int)(4.0 * G4UniformRand());
  G4ThreeVector local((2.0 * G4UniformRand() - 1.0) * kHalf[0],
                      (2.0 * G4UniformRand() - 1.0) * kHalf[1],
                      (2.0 * G4UniformRand() - 1.0) * kHalf[2]);
  return CrystalCenter(copy) + local;
}

void PrimaryGenerator::Fire(G4Event *anEvent, G4ParticleDefinition *particle,
                            G4double energy, const G4ThreeVector &position) {
  fGun->SetParticleDefinition(particle);
  fGun->SetParticleEnergy(energy);
  fGun->SetParticlePosition(position);
  fGun->SetParticleMomentumDirection(G4RandomDirection());
  fGun->GeneratePrimaryVertex(anEvent);
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  if (fMode == "beam") {
    fGun->SetParticleDefinition(G4Gamma::Gamma());
    fGun->SetParticleEnergy(kLineEnergy);
    fGun->SetParticlePosition(CrystalCenter(3) +
                              G4ThreeVector(0, 0, kHalf[2] + 3.0 * mm));
    fGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, -1));
    fGun->GeneratePrimaryVertex(anEvent);
    return;
  }
  G4ThreeVector position =
      (fMode == "uniform") ? UniformPoint() : SurfaceCapturePoint();
  G4double photon_fraction = 1.0 / (1.0 + kAlphaTotal);
  G4double k_fraction = kAlphaK / (1.0 + kAlphaTotal);
  G4double pick = fConversion ? G4UniformRand() : 0.0;
  if (pick < photon_fraction) {
    Fire(anEvent, G4Gamma::Gamma(), kLineEnergy, position);
  } else if (pick < photon_fraction + k_fraction) {
    Fire(anEvent, G4Electron::Electron(), kLineEnergy - kCdKBinding, position);
    if (G4UniformRand() < kOmegaK) {
      G4double xray = PickCdXray();
      Fire(anEvent, G4Gamma::Gamma(), xray, position);
      Fire(anEvent, G4Electron::Electron(), kCdKBinding - xray, position);
    } else {
      Fire(anEvent, G4Electron::Electron(), kCdKBinding, position);
    }
  } else {
    Fire(anEvent, G4Electron::Electron(), kLineEnergy - kCdLBinding, position);
    Fire(anEvent, G4Electron::Electron(), kCdLBinding, position);
  }
}
