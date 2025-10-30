#ifndef G4LivermorePolarizedPhotoElectricGDModel_h
#define G4LivermorePolarizedPhotoElectricGDModel_h 1

#include "G4VEmModel.hh"
#include "G4ParticleChangeForGamma.hh"

class G4LivermorePolarizedPhotoElectricGDModel : public G4VEmModel {
public:
  explicit G4LivermorePolarizedPhotoElectricGDModel(
      const G4String& name = "LivermorePolPE_GD");
  ~G4LivermorePolarizedPhotoElectricGDModel() override = default;

  void Initialise(const G4ParticleDefinition*, const G4DataVector&) override;
  void SampleSecondaries(std::vector<G4DynamicParticle*>*,
                         const G4MaterialCutsCouple*,
                         const G4DynamicParticle*,
                         G4double tCut,
                         G4double maxEnergy) override;

private:
  // in old code this was a value, but in your Geant4 we should really
  // grab it via GetParticleChangeForGamma() in Initialise()
  G4ParticleChangeForGamma* fParticleChange = nullptr;
};

#endif
