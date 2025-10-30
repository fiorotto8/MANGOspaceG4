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

namespace B3
{

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event*) override;

    const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

  private:
    // Hard-coded gun (particle/direction/position logic stays here)
    G4ParticleGun* fParticleGun = nullptr;

    // One row of the input spectrum
    struct SpectrumBin {
      G4double low_keV;
      G4double high_keV;
      G4double weight;     // counts
      G4double polMean;    // "polarization"
      G4double polSigma;   // "polarization_error"
    };

    std::vector<SpectrumBin> fBins;
    std::vector<G4double>    fCdf;
    G4double                 fTotalW = 0.0;

    void LoadSpectrum(const G4String& filename);

    // sample energy and get the (mean, sigma) of polarization for that bin
    G4double SampleEnergyFromSpectrum(G4double& outPolMean,
                                      G4double& outPolSigma) const;

    // from bin's (mean, sigma) → event polarization vector
    G4ThreeVector SamplePolarizationVector(G4double polMean,
                                          G4double polSigma) const;
};

} // namespace B3

#endif
