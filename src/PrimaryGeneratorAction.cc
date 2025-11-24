/// \file B3/B3a/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B3::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"         // G4UniformRand, G4RandGauss
#include "G4Exception.hh"
#include "G4ExceptionSeverity.hh"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>            // std::replace
#include <vector>

namespace B3 {

PrimaryGeneratorAction::PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction(),
    fEmissionMode(kFixedDirection),
    fSphereRadius(50.0 * cm),
    fParticleGun(nullptr),
    fMessenger(nullptr),
    fTotalW(0.0)
{
  // Particle gun (default: gamma)
  fParticleGun = new G4ParticleGun(1);
  auto* table  = G4ParticleTable::GetParticleTable();
  fParticleGun->SetParticleDefinition(table->FindParticle("gamma"));

  // placeholders (will be overwritten per event)
  fParticleGun->SetParticleEnergy(1. * keV);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
  fParticleGun->SetParticlePolarization(G4ThreeVector(0., 1., 0.));

  // default spectrum (can be overridden from macro)
  LoadSpectrum("../spectra/55Fe.txt");

  // UI messenger
  fMessenger = new PrimaryGeneratorMessenger(this);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
  delete fMessenger;
}

// --------------------------------------------------
// Change particle type by name (used by UI)
// --------------------------------------------------
void PrimaryGeneratorAction::SetParticleName(const G4String& name)
{
  auto* table = G4ParticleTable::GetParticleTable();
  auto* p     = table->FindParticle(name);
  if (!p) {
    G4Exception("PrimaryGeneratorAction::SetParticleName",
                "B3_UNKNOWN_PARTICLE", JustWarning,
                "Unknown particle name; keeping old one.");
    return;
  }
  fParticleGun->SetParticleDefinition(p);
}

// --------------------------------------------------
// LoadSpectrum: supports TWO formats.
//
// (A) "Line" format: 8 columns (as from your Python generator)
//     bin  low_keV  center_keV  high_keV  counts  error  pol  polErr
//
// (B) Flux CSV: 2 columns
//     E[MeV], Phi(E) [particles cm^-2 s^-1 sr^-1 MeV^-1]
//
// In (A), weight = counts (unchanged behaviour).
// In (B), we construct bins in E and set weight_i = Phi(E_i) * ΔE_i
// so that probabilities are ∝ ∫ Phi(E) dE, consistent with (A).
// --------------------------------------------------
void PrimaryGeneratorAction::LoadSpectrum(const G4String& filename)
{
  std::ifstream fin(filename);
  if (!fin.is_open()) {
    G4String msg = "Cannot open spectrum file: " + filename +
                  "\n- Check the path (it is relative to the run directory)"
                  "\n- Check permissions"
                  "\n- Check that the file name is correct";
    G4Exception("PrimaryGeneratorAction::LoadSpectrum",
                "B3_SPECTRUM_FILE_NOT_FOUND",
                FatalException,
                msg);
    return; // not reached
  }

  fBins.clear();
  fCdf.clear();
  fTotalW = 0.0;

  // --- 1) Detect format from first data line ---
  std::string line;
  std::string firstDataLine;
  bool hasDataLine = false;

  while (std::getline(fin, line)) {
    if (line.empty() || line[0] == '#') continue;
    firstDataLine = line;
    hasDataLine   = true;
    break;
  }

  if (!hasDataLine) {
    G4cerr << "[LoadSpectrum] WARNING: file " << filename
           << " has no data lines.\n";
    return;
  }

  // Replace commas with spaces to allow both comma and space separation
  std::replace(firstDataLine.begin(), firstDataLine.end(), ',', ' ');

  std::istringstream issDetect(firstDataLine);
  std::vector<G4double> tokens;
  {
    G4double v;
    while (issDetect >> v) {
      tokens.push_back(v);
    }
  }

  bool isEightColumn = (tokens.size() >= 8);
  bool isTwoColumn   = (tokens.size() == 2);

  if (!isEightColumn && !isTwoColumn) {
    G4cerr << "[LoadSpectrum] ERROR: Cannot detect spectrum format in "
           << filename << " (tokens in first data line = " << tokens.size()
           << "). Expect 8 or 2 numbers.\n";
    return;
  }

  // Rewind file to beginning to parse fully
  fin.clear();
  fin.seekg(0);

  // --------------------------------------------------
  // (A) 8-column "line spectrum" format
  // --------------------------------------------------
  if (isEightColumn) {
    while (std::getline(fin, line)) {
      if (line.empty() || line[0] == '#') continue;

      std::replace(line.begin(), line.end(), ',', ' ');
      std::istringstream iss(line);

      int      binid;
      G4double low, center, high, counts, err, pol, polErr;
      if (!(iss >> binid >> low >> center >> high >> counts >> err >> pol >> polErr)) {
        G4cerr << "[LoadSpectrum] Malformed 8-column line in " << filename
               << ": '" << line << "'\n";
        continue;
      }

      SpectrumBin b;
      b.low_keV  = low;
      b.high_keV = high;
      b.weight   = counts;   // <- unchanged: counts are the weights
      b.polMean  = pol;
      b.polSigma = polErr;

      fBins.push_back(b);
    }
  }
  // --------------------------------------------------
  // (B) 2-column flux CSV: E[MeV], Phi(E)
  // --------------------------------------------------
  else if (isTwoColumn) {
    std::vector<G4double> E_MeV;
    std::vector<G4double> Phi;

    while (std::getline(fin, line)) {
      if (line.empty() || line[0] == '#') continue;

      std::replace(line.begin(), line.end(), ',', ' ');
      std::istringstream iss(line);

      G4double eMeV, flux;
      if (!(iss >> eMeV >> flux)) {
        G4cerr << "[LoadSpectrum] Malformed 2-column line in " << filename
               << ": '" << line << "'\n";
        continue;
      }

      E_MeV.push_back(eMeV);
      Phi.push_back(flux);
    }

    if (E_MeV.size() < 2) {
      G4cerr << "[LoadSpectrum] WARNING: file " << filename
             << " has fewer than 2 valid points; cannot build bins.\n";
      return;
    }

    const size_t N = E_MeV.size();
    // Build bins around each sampled energy using midpoints
    for (size_t i = 0; i < N; ++i) {
      G4double E   = E_MeV[i];
      G4double lowE, highE;

      if (i == 0) {
        // First point: extrapolate lower edge
        G4double dE = E_MeV[1] - E_MeV[0];
        lowE        = E - 0.5 * dE;
      } else {
        lowE = 0.5 * (E_MeV[i-1] + E_MeV[i]);
      }

      if (i == N - 1) {
        // Last point: extrapolate upper edge
        G4double dE = E_MeV[N-1] - E_MeV[N-2];
        highE       = E + 0.5 * dE;
      } else {
        highE = 0.5 * (E_MeV[i] + E_MeV[i+1]);
      }

      G4double dE = highE - lowE;      // MeV
      if (dE <= 0.) continue;

      SpectrumBin b;
      // convert MeV -> keV for the Geant4 side
      b.low_keV  = lowE * 1000.0;
      b.high_keV = highE * 1000.0;
      // weight ∝ Φ(E) * ΔE (integrated flux over that bin)
      b.weight   = Phi[i] * dE;
      b.polMean  = 0.0;   // backgrounds assumed unpolarized
      b.polSigma = 0.0;

      fBins.push_back(b);
    }
  }

  // --- build CDF from weights ---
  G4double cum = 0.0;
  fCdf.clear();
  fCdf.reserve(fBins.size());
  for (const auto& b : fBins) {
    cum += (b.weight > 0.0) ? b.weight : 0.0;
    fCdf.push_back(cum);
  }
  fTotalW = cum;

  if (fTotalW <= 0.0) {
    G4cerr << "[PrimaryGeneratorAction::LoadSpectrum] WARNING: spectrum "
           << filename << " has zero total weight; default energy will be used.\n";
  } else {
    G4cout << "[PrimaryGeneratorAction::LoadSpectrum] Loaded spectrum from "
           << filename << " with " << fBins.size()
           << " bins, total weight = " << fTotalW << G4endl;
  }
}

// --------------------------------------------------
// Pick a bin using the weights as probabilities.
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
// Polarization sampling (unchanged)
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

// --------------------------------------------------
// GeneratePrimaries: fixed disk or isotropic sphere
// --------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // 1) energy & polarization params from spectrum
  G4double polMean  = 0.0;
  G4double polSigma = 0.0;
  G4double e_keV    = SampleEnergyFromSpectrum(polMean, polSigma);
  fParticleGun->SetParticleEnergy(e_keV * keV);

  // 2) position + direction distribution depending on mode
  if (fEmissionMode == kFixedDirection) {

    // original disk source, beam along +z
    const G4double r      = 1.25 * cm;
    const G4double theta  = G4UniformRand() * 2.0 * CLHEP::pi;
    const G4double radius = std::sqrt(G4UniformRand()) * r;
    const G4double x      = radius * std::cos(theta);
    const G4double y      = radius * std::sin(theta);
    fParticleGun->SetParticlePosition(G4ThreeVector(x, y, -1. * cm));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

  } else if (fEmissionMode == kIsotropicSphere) {

    // isotropic from a sphere of radius fSphereRadius around the detector
    const G4double R = fSphereRadius;

    // random direction on 4π
    G4double u   = 2.0 * G4UniformRand() - 1.0;  // cos(theta) in [-1,1]
    G4double phi = 2.0 * CLHEP::pi * G4UniformRand();
    G4double sinTheta = std::sqrt(1.0 - u*u);
    G4double nx = sinTheta * std::cos(phi);
    G4double ny = sinTheta * std::sin(phi);
    G4double nz = u;

    G4ThreeVector n(nx, ny, nz);

    // vertex on sphere surface, momentum inward
    fParticleGun->SetParticlePosition(-R * n);
    fParticleGun->SetParticleMomentumDirection(n);
  }

  // 3) polarization for this event
  G4ThreeVector polVec = SamplePolarizationVector(polMean, polSigma);
  fParticleGun->SetParticlePolarization(polVec);

  // 4) shoot
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

} // namespace B3
