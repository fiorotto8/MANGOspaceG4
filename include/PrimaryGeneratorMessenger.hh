/// \file B3/B3a/include/PrimaryGeneratorMessenger.hh

#ifndef B3PrimaryGeneratorMessenger_h
#define B3PrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

namespace B3 {

class PrimaryGeneratorAction;

class PrimaryGeneratorMessenger : public G4UImessenger
{
  public:
    PrimaryGeneratorMessenger(PrimaryGeneratorAction* gun);
    ~PrimaryGeneratorMessenger() override;

    void SetNewValue(G4UIcommand* cmd, G4String value) override;

  private:
    PrimaryGeneratorAction*    fAction        = nullptr;
    G4UIdirectory*             fDir           = nullptr;
    G4UIcmdWithAString*        fSpectrumCmd   = nullptr;
    G4UIcmdWithAString*        fParticleCmd   = nullptr;
    G4UIcmdWithAString*        fModeCmd       = nullptr;
    G4UIcmdWithADoubleAndUnit* fSphereRadCmd  = nullptr;
};

} // namespace B3

#endif // B3PrimaryGeneratorMessenger_h
