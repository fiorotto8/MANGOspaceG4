/// \file B3/B3a/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B3::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"     // G4UniformRand, G4RandGauss

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

namespace B3
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  // 1) HARD-CODED PARTICLE
  // change "gamma" to "proton", "e-", ... if you need
  fParticleGun = new G4ParticleGun(1);
  auto* table  = G4ParticleTable::GetParticleTable();
  fParticleGun->SetParticleDefinition(table->FindParticle("gamma"));

  // placeholders (will be overwritten per event)
  fParticleGun->SetParticleEnergy(1. * keV);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
  fParticleGun->SetParticlePolarization(G4ThreeVector(0., 1., 0.));

  // 2) load your spectrum
  // NOTE: path is relative to run dir
  LoadSpectrum("../analysis/spectrum.txt");
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

// --------------------------------------------------
// File format:
// #bin  bin_low_keV  bin_center_keV  bin_high_keV
//      counts  error  polarization  polarization_error
// --------------------------------------------------
void PrimaryGeneratorAction::LoadSpectrum(const G4String& filename)
{
  std::ifstream fin(filename);
  if (!fin.is_open()) {
    G4cerr << "[PrimaryGeneratorAction] ERROR: cannot open " << filename << G4endl;
    return;
  }

  fBins.clear();
  fCdf.clear();
  fTotalW = 0.0;

  std::string line;
  while (std::getline(fin, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::istringstream iss(line);
    int binid;
    G4double low, center, high, counts, err, pol, polErr;
    if (!(iss >> binid >> low >> center >> high >> counts >> err >> pol >> polErr)) {
      continue;
    }

    SpectrumBin b;
    b.low_keV  = low;
    b.high_keV = high;
    b.weight   = counts;
    b.polMean  = pol;
    b.polSigma = polErr;

    fBins.push_back(b);
  }

  // build CDF from counts
  G4double cum = 0.0;
  fCdf.reserve(fBins.size());
  for (const auto& b : fBins) {
    cum += (b.weight > 0.0) ? b.weight : 0.0;
    fCdf.push_back(cum);
  }
  fTotalW = cum;

  if (fTotalW <= 0.0) {
    G4cerr << "[PrimaryGeneratorAction] WARNING: spectrum " << filename
          << " has zero total counts; default energy will be used.\n";
  }
}

// --------------------------------------------------
// Pick a bin using the counts as weights.
// Return an energy (keV), and the pol mean/sigma of that bin.
// --------------------------------------------------
G4double PrimaryGeneratorAction::SampleEnergyFromSpectrum(G4double& outPolMean,
                                                          G4double& outPolSigma) const
{
  // fallback
  if (fBins.empty() || fTotalW <= 0.0) {
    outPolMean  = 0.0;
    outPolSigma = 0.0;
    return 17.4; // keV
  }

  // choose a bin by CDF
  G4double u = G4UniformRand() * fTotalW;
  size_t idx = 0;
  for (; idx < fCdf.size(); ++idx) {
    if (u <= fCdf[idx]) break;
  }
  if (idx >= fBins.size()) idx = fBins.size() - 1;

  const auto& b = fBins[idx];

  // sample uniformly inside the bin
  G4double e_keV = b.low_keV + G4UniformRand() * (b.high_keV - b.low_keV);
  if (e_keV < 0.) e_keV = 0.001;   // protect from tiny negatives

  outPolMean  = b.polMean;
  outPolSigma = b.polSigma;

  return e_keV;
}

// --------------------------------------------------
// For THIS event:
//   p ~ N(polMean, polSigma)
//   clamp p to [0,1]
//   with prob p → (0,1,0)
//   else → (0,0,0)
// --------------------------------------------------
G4ThreeVector PrimaryGeneratorAction::SamplePolarizationVector(G4double polMean,
                                                              G4double polSigma) const
{
  // draw p from Gaussian
  G4double p = polMean;
  if (polSigma > 0.) {
    p = G4RandGauss::shoot(polMean, polSigma);
  }

  // clamp
  if (p < 0.) p = 0.;
  if (p > 1.) p = 1.;

  // Bernoulli
  if (G4UniformRand() <= p) {
    // linearly polarized along Y (hard-coded)
    return G4ThreeVector(0., 1., 0.);
  }

  // unpolarized
  return G4ThreeVector();
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // 1) energy & polarization params from spectrum
  G4double polMean  = 0.0;
  G4double polSigma = 0.0;
  G4double e_keV    = SampleEnergyFromSpectrum(polMean, polSigma);
  fParticleGun->SetParticleEnergy(e_keV * keV);

  // 2) HARD-CODED position distribution (your disk)
  const G4double r = 3.7 * cm;
  const G4double theta  = G4UniformRand() * 2.0 * CLHEP::pi;
  const G4double radius = std::sqrt(G4UniformRand()) * r;
  const G4double x = radius * std::cos(theta);
  const G4double y = radius * std::sin(theta);
  fParticleGun->SetParticlePosition(G4ThreeVector(x, y, -1 * cm));

  // 3) HARD-CODED direction
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

  // 4) polarization for this event
  G4ThreeVector polVec = SamplePolarizationVector(polMean, polSigma);
  fParticleGun->SetParticlePolarization(polVec);

  // 5) shoot
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

} // namespace B3
