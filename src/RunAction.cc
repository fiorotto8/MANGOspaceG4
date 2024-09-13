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

    analysisManager->CreateNtuple("nTuple","nTuple");
    analysisManager->CreateNtupleIColumn("EventNumber");
    analysisManager->CreateNtupleSColumn("ParticleName");
    analysisManager->CreateNtupleIColumn("ParticleID");
    analysisManager->CreateNtupleIColumn("ParticleTag");
    analysisManager->CreateNtupleIColumn("ParentID");
    analysisManager->CreateNtupleDColumn("x_hits");
    analysisManager->CreateNtupleDColumn("y_hits");
    analysisManager->CreateNtupleDColumn("z_hits");
    analysisManager->CreateNtupleDColumn("EnergyDeposit");
    analysisManager->CreateNtupleIColumn("VolumeNumber");
    analysisManager->CreateNtupleDColumn("VolumeTraslX");
    analysisManager->CreateNtupleDColumn("VolumeTraslY");
    analysisManager->CreateNtupleDColumn("VolumeTraslZ");
    analysisManager->CreateNtupleSColumn("ProcessType");
    analysisManager->CreateNtupleDColumn("px_particle");
    analysisManager->CreateNtupleDColumn("py_particle");
    analysisManager->CreateNtupleDColumn("pz_particle");
    analysisManager->CreateNtupleIColumn("pdgID_hits");
    analysisManager->CreateNtupleDColumn("tracklen_hits");
    analysisManager->CreateNtupleIColumn("currentTrackID");//trackID for later
    analysisManager->CreateNtupleDColumn("EnergyInitial");//trackID for later


    analysisManager->FinishNtuple(0);

}

void RunAction::EndOfRunAction(const G4Run*) {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}