#include "G4LivermorePolarizedPhotoElectricGDModel.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4DynamicParticle.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

G4LivermorePolarizedPhotoElectricGDModel::
G4LivermorePolarizedPhotoElectricGDModel(const G4String& name)
  : G4VEmModel(name)
{
  // energy limits if you want them, not strictly needed
  SetLowEnergyLimit(10*eV);
  SetHighEnergyLimit(100*GeV);
}

void G4LivermorePolarizedPhotoElectricGDModel::Initialise(
    const G4ParticleDefinition* part,
    const G4DataVector&)
{
  // we only care about gammas
  if (part == G4Gamma::Gamma()) {
    // this is the correct API for recent Geant4
    fParticleChange = GetParticleChangeForGamma();
  }
}

void G4LivermorePolarizedPhotoElectricGDModel::SampleSecondaries(
    std::vector<G4DynamicParticle*>* fvect,
    const G4MaterialCutsCouple* /*couple*/,
    const G4DynamicParticle* aDynamicGamma,
    G4double /*tCut*/,
    G4double /*maxEnergy*/)
{
  const G4double k = aDynamicGamma->GetKineticEnergy();

  // kill the incoming gamma
  fParticleChange->SetProposedKineticEnergy(0.);
  fParticleChange->ProposeTrackStatus(fStopAndKill);

  if (k <= 0.) {
    return;
  }

  // for now: all photon energy goes to the electron
  const G4double elecE = k;

  // photon direction
  const G4ThreeVector photonDir = aDynamicGamma->GetMomentumDirection();

  // ---- GD-like angular sampling (simplified, from your attempt) ----
  G4double theta, phi;
  const G4double beta = elecE / (elecE + electron_mass_c2);
  const G4double eTot = elecE + electron_mass_c2;

  while (true) {
    const G4double u = G4UniformRand();
    theta = std::acos(1. - 2.*u);
    phi   = twopi * G4UniformRand();

    const G4double oneMinusBcos = 1. - beta*std::cos(theta);
    const G4double sin2t = std::sin(theta)*std::sin(theta);
    const G4double cos2p = std::cos(phi)*std::cos(phi);

    const G4double num =
      sin2t / std::pow(oneMinusBcos, 4) *
      ( 0.25*k*k*oneMinusBcos + ((1./eTot) - k*k*oneMinusBcos)*cos2p );

    if (num > 0. && G4UniformRand() < num) break;
  }

  // electron direction in photon frame
  G4ThreeVector eDir(std::sin(theta)*std::cos(phi),
                     std::sin(theta)*std::sin(phi),
                     std::cos(theta));
  eDir.rotateUz(photonDir);

  auto* electron = new G4DynamicParticle(G4Electron::Electron(), eDir, elecE);
  fvect->push_back(electron);

  // if you later subtract binding energy, add a local deposit here
  // fParticleChange->ProposeLocalEnergyDeposit(bindingE);
}
