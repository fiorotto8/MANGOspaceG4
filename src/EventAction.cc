#include "EventAction.hh"
#include "GasBoxSensitiveDetector.hh"
#include <G4SDManager.hh>
#include <map>
#include <vector>
#include <string>


EventAction::EventAction() : G4UserEventAction() {
    // Constructor implementation
}

EventAction::~EventAction() {
    // Destructor implementation (can be empty)
}

void EventAction::BeginOfEventAction(const G4Event*) {
    // Implementation of BeginOfEventAction (can be empty)
}

void EventAction::EndOfEventAction(const G4Event* event) {
    G4SDManager* SDManager = G4SDManager::GetSDMpointer();
    GasBoxSensitiveDetector* gasBoxSD = static_cast<GasBoxSensitiveDetector*>(SDManager->FindSensitiveDetector("GasBoxSD"));


    if (gasBoxSD) {
        //PRIMRIES
        BoolArrived.push_back(gasBoxSD->HasParticleArrived());
        BoolInteracted.push_back(gasBoxSD->HasParticleInteracted());
        totalPrimaryEnergyDeposit.push_back(gasBoxSD->GetTotPrimEnergyDeposit());
        primaryEnergy.push_back(gasBoxSD->GetPrimaryEnergy());
        primaryPDG.push_back(gasBoxSD->GetPDGPrimary());
        //Secondaries
        numSecondaries.push_back(gasBoxSD->GetNumSecondaries());
        // Create an instance of the custom class
        std::vector<std::vector<int>> secondariesPDG;
        secondariesPDG.push_back(gasBoxSD->GetPDGSecondaries());
    }
}