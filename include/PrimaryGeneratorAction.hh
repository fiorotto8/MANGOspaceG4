/// \file B3/B3a/include/PrimaryGeneratorAction.hh
/// \brief Definition of the B3::PrimaryGeneratorAction class

#ifndef B3PrimaryGeneratorAction_h
#define B3PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

#include <vector>

class G4ParticleGun;
class G4Event;

namespace B3 {

class PrimaryGeneratorMessenger;  // forward declaration

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

    const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

    // UI helpers
    void SetSpectrumFile(const G4String& fname) { LoadSpectrum(fname); }
    void SetParticleName(const G4String& name);

    // Only two modes: fixed disk beam, or isotropic sphere
    enum EmissionMode { kFixedDirection, kIsotropicSphere };
    void SetEmissionMode(EmissionMode m) { fEmissionMode = m; }

    void SetSphereRadius(G4double r) { fSphereRadius = r; }

  private:
    // emission configuration
    EmissionMode               fEmissionMode;
    G4double                   fSphereRadius;   // radius of emission sphere

    G4ParticleGun*             fParticleGun;
    PrimaryGeneratorMessenger* fMessenger;

    // One row of the input spectrum
    struct SpectrumBin {
      G4double low_keV;
      G4double high_keV;
      G4double weight;     // counts (∝ flux * ΔE)
      G4double polMean;    // "polarization"
      G4double polSigma;   // "polarization_error"
    };

    std::vector<SpectrumBin> fBins;
    std::vector<G4double>    fCdf;
    G4double                 fTotalW;

    void         LoadSpectrum(const G4String& filename);
    G4double     SampleEnergyFromSpectrum(G4double& outPolMean,
                                          G4double& outPolSigma) const;
    G4ThreeVector SamplePolarizationVector(G4double polMean,
                                           G4double polSigma) const;
};

} // namespace B3

#endif // B3PrimaryGeneratorAction_h
