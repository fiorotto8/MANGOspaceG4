#include "RunAction.hh"
#include "EventAction.hh"
#include <vector>
#include <iostream>

RunAction::RunAction(EventAction* eventAction) 
    : G4UserRunAction(), eventAction(eventAction), outputFile(nullptr), tree(nullptr) {
    // If eventAction is nullptr, handle accordingly
    // (e.g., not creating branches in the TTree)
}

RunAction::~RunAction() {
    delete outputFile;
}

void RunAction::BeginOfRunAction(const G4Run*) {
    // No action needed at run start
}

void RunAction::EndOfRunAction(const G4Run*) {
    outputFile = new TFile("output.root", "RECREATE");
    tree = new TTree("Tree", "Geant4 Data");
//PRIMRIES
    std::vector<G4bool> arrived = eventAction->GetArrivedBooleans();
    std::vector<G4bool> interacted = eventAction->GetInteractedBooleans();
    std::vector<G4double> energyDeposit = eventAction->GetTotEnergyDeposit();
    std::vector<G4double> primaryEnergy = eventAction->GetPrimaryEnergy();
    std::vector<G4int> primaryPDG = eventAction->GetPDGPrimary();
//SECONDARIES
    std::vector<G4int> numSecondaries = eventAction->GetNumSecondaries();
    std::vector<std::vector<int>> secondariesPDG = eventAction->GetPDGSecondaries();

//PRIMRIES
    tree->Branch("BoolArrived", &arrived);
    tree->Branch("BoolInteracted", &interacted);
    tree->Branch("energyDeposit", &energyDeposit);
    tree->Branch("primaryEnergy", &primaryEnergy);
    tree->Branch("primaryPDG", &primaryPDG);
//SECONDARIES
    tree->Branch("numSecondaries", &numSecondaries);
    tree->Branch("secondariesPDG", &secondariesPDG);

    tree->Fill();

    tree->Write();
    outputFile->Close();
}