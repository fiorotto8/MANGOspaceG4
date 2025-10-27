#include "GasBoxSensitiveDetector.hh"

#include "EventAction.hh"

#include <G4RunManager.hh>
#include <G4Step.hh>
#include <G4StepPoint.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4TouchableHistory.hh>

GasBoxSensitiveDetector::GasBoxSensitiveDetector(const G4String& name)
    : G4VSensitiveDetector(name) {}

GasBoxSensitiveDetector::~GasBoxSensitiveDetector() = default;

G4bool GasBoxSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory* /*Rohist*/)
{
    auto* eventAction = static_cast<EventAction*>(G4RunManager::GetRunManager()->GetUserEventAction());
    if (!eventAction) {
        return false;
    }

    auto* track = aStep->GetTrack();
    auto* preStepPoint = aStep->GetPreStepPoint();

    const auto position = preStepPoint->GetPosition();
    const auto* particleDef = track->GetDefinition();
    const G4String particleName = particleDef->GetParticleName();

    G4int volumeCopyNumber = -1;
    G4ThreeVector volumeTranslation(0., 0., 0.);
    const auto touchable = preStepPoint->GetTouchableHandle();
    if (touchable) {
        volumeCopyNumber = touchable->GetCopyNumber();
        volumeTranslation = touchable->GetTranslation();
    }

    G4String processType("Primary");
    if (const auto* creator = track->GetCreatorProcess()) {
        processType = creator->GetProcessName();
    }

    const G4int pdgID = particleDef->GetPDGEncoding();
    G4int particleTag = -1;
    if (pdgID == 11) {
        particleTag = 0;
    } else if (pdgID == -11) {
        particleTag = 1;
    } else if (pdgID == 22) {
        particleTag = 2;
    } else if (pdgID == 2212) {
        particleTag = 3;
    }

    EventAction::HitRecord record;
    record.particleName = particleName;
    record.trackID = track->GetTrackID();
    record.particleTag = particleTag;
    record.parentID = track->GetParentID();
    record.position = position;
    record.energyDeposit = aStep->GetTotalEnergyDeposit();
    record.volumeCopyNumber = volumeCopyNumber;
    record.volumeTranslation = volumeTranslation;
    record.processType = processType;
    record.momentumDirection = track->GetMomentumDirection();
    record.pdgID = pdgID;
    record.stepLength = aStep->GetStepLength();
    record.initialEnergy = track->GetVertexKineticEnergy();

    eventAction->AddHit(record);

    return true;
}
