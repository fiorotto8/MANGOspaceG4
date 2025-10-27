#include "EventAction.hh"
#include "GasBoxSensitiveDetector.hh"
#include "RunAction.hh"
#include <G4SDManager.hh>
#include <G4AnalysisManager.hh>
#include <map>
#include <vector>
#include <string>
#include <G4Event.hh>

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
    G4int evtNb = event->GetEventID(); 
    if(evtNb%10000==0){
        G4cout << evtNb << G4endl;
    }

    G4SDManager* SDManager = G4SDManager::GetSDMpointer();
    GasBoxSensitiveDetector* gasBoxSD = static_cast<GasBoxSensitiveDetector*>(SDManager->FindSensitiveDetector("GasBoxSD"));
    // Get the instance of the run manager
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
}