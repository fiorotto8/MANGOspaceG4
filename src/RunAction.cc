#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "EventAction.hh"
#include <vector>
#include <iostream>

RunAction::RunAction(EventAction* eventAction)
    : G4UserRunAction(), eventAction(eventAction){
}

RunAction::~RunAction() {
}

void RunAction::BeginOfRunAction(const G4Run*) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile("output.root");

    analysisManager->CreateNtuple("tree","tree");
    analysisManager->CreateNtupleIColumn("BoolArrived");
    analysisManager->CreateNtupleIColumn("BoolInteracted");
    analysisManager->CreateNtupleDColumn("energyDeposit");
    analysisManager->CreateNtupleDColumn("primaryEnergy");
    analysisManager->CreateNtupleIColumn("primaryPDG");
    analysisManager->CreateNtupleIColumn("primaryHits");
    analysisManager->CreateNtupleIColumn("secondaryHits");

    analysisManager->FinishNtuple(0);

}

void RunAction::EndOfRunAction(const G4Run*) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}