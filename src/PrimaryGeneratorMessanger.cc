/// \file B3/B3a/src/PrimaryGeneratorMessenger.cc

#include "PrimaryGeneratorMessenger.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

namespace B3 {

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* gun)
    : fAction(gun)
{
    fDir = new G4UIdirectory("/B3/primary/");
    fDir->SetGuidance("Controls for primary generator");

    fSpectrumCmd = new G4UIcmdWithAString("/B3/primary/spectrumFile", this);
    fSpectrumCmd->SetGuidance("Path to spectrum file");
    fSpectrumCmd->SetParameterName("filename", false);

    fParticleCmd = new G4UIcmdWithAString("/B3/primary/particle", this);
    fParticleCmd->SetGuidance("G4 particle name (gamma, e-, proton, ...)");
    fParticleCmd->SetParameterName("pname", false);

    fModeCmd = new G4UIcmdWithAString("/B3/primary/emissionMode", this);
    fModeCmd->SetGuidance("Emission type: fixed disk or isotropic sphere");
    fModeCmd->SetParameterName("mode", false);
    fModeCmd->SetCandidates("fixed sphere");

    fSphereRadCmd = new G4UIcmdWithADoubleAndUnit("/B3/primary/sphereRadius", this);
    fSphereRadCmd->SetGuidance("Radius of isotropic emission sphere");
    fSphereRadCmd->SetParameterName("R", false);
    fSphereRadCmd->SetDefaultUnit("cm");
    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger()
    {
    delete fSpectrumCmd;
    delete fParticleCmd;
    delete fModeCmd;
    delete fSphereRadCmd;
    delete fDir;
    }

    void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* cmd, G4String value)
    {
    if (cmd == fSpectrumCmd) {

    fAction->SetSpectrumFile(value);

    } else if (cmd == fParticleCmd) {

    fAction->SetParticleName(value);

    } else if (cmd == fModeCmd) {

    if (value == "fixed") {
        fAction->SetEmissionMode(PrimaryGeneratorAction::kFixedDirection);
    } else if (value == "sphere") {
        fAction->SetEmissionMode(PrimaryGeneratorAction::kIsotropicSphere);
    }

    } else if (cmd == fSphereRadCmd) {

    G4double R = fSphereRadCmd->GetNewDoubleValue(value);
    fAction->SetSphereRadius(R);

    }
}

} // namespace B3
