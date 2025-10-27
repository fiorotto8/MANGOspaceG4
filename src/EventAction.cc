#include "EventAction.hh"

#include <G4AnalysisManager.hh>
#include <G4Event.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ios.hh>

EventAction::EventAction()
    : G4UserEventAction(), fHitRecords(), fEventID(-1), fPrimaryName(""),
      fPrimaryPDG(0), fPrimaryEnergy(0.) {}

EventAction::~EventAction() = default;

void EventAction::BeginOfEventAction(const G4Event* event) {
    fHitRecords.clear();
    fEventID = event->GetEventID();
    fPrimaryName = "";
    fPrimaryPDG = 0;
    fPrimaryEnergy = 0.;

    if (event->GetNumberOfPrimaryVertex() > 0) {
        const auto* vertex = event->GetPrimaryVertex(0);
        if (vertex && vertex->GetNumberOfParticle() > 0) {
            const auto* primary = vertex->GetPrimary(0);
            if (primary) {
                const auto* definition = primary->GetParticleDefinition();
                if (definition) {
                    fPrimaryName = definition->GetParticleName();
                    fPrimaryPDG = definition->GetPDGEncoding();
                }
                fPrimaryEnergy = primary->GetKineticEnergy();
            }
        }
    }
}

void EventAction::EndOfEventAction(const G4Event* /*event*/) {
    if (fEventID >= 0 && (fEventID % 10000) == 0) {
        G4cout << fEventID << G4endl;
    }

    auto* analysisManager = G4AnalysisManager::Instance();
    for (const auto& hit : fHitRecords) {
        analysisManager->FillNtupleIColumn(0, fEventID);
        analysisManager->FillNtupleSColumn(1, hit.particleName);
        analysisManager->FillNtupleIColumn(2, hit.trackID);
        analysisManager->FillNtupleIColumn(3, hit.particleTag);
        analysisManager->FillNtupleIColumn(4, hit.parentID);
        analysisManager->FillNtupleDColumn(5, hit.position.x());
        analysisManager->FillNtupleDColumn(6, hit.position.y());
        analysisManager->FillNtupleDColumn(7, hit.position.z());
        analysisManager->FillNtupleDColumn(8, hit.energyDeposit);
        analysisManager->FillNtupleIColumn(9, hit.volumeCopyNumber);
        analysisManager->FillNtupleDColumn(10, hit.volumeTranslation.x());
        analysisManager->FillNtupleDColumn(11, hit.volumeTranslation.y());
        analysisManager->FillNtupleDColumn(12, hit.volumeTranslation.z());
        analysisManager->FillNtupleSColumn(13, hit.processType);
        analysisManager->FillNtupleDColumn(14, hit.momentumDirection.x());
        analysisManager->FillNtupleDColumn(15, hit.momentumDirection.y());
        analysisManager->FillNtupleDColumn(16, hit.momentumDirection.z());
        analysisManager->FillNtupleIColumn(17, hit.pdgID);
        analysisManager->FillNtupleDColumn(18, hit.stepLength);
        analysisManager->FillNtupleIColumn(19, hit.trackID);
        analysisManager->FillNtupleDColumn(20, hit.initialEnergy);
        analysisManager->FillNtupleSColumn(21, fPrimaryName);
        analysisManager->FillNtupleIColumn(22, fPrimaryPDG);
        analysisManager->FillNtupleDColumn(23, fPrimaryEnergy);
        analysisManager->AddNtupleRow(0);
    }
}

void EventAction::AddHit(const HitRecord& record) {
    fHitRecords.push_back(record);
}