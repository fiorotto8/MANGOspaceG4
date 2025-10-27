#include "RunAction.hh"

#include "G4AnalysisManager.hh"

RunAction::RunAction(EventAction* /*eventAction*/)
    : G4UserRunAction(), fNtupleBooked(false) {}

RunAction::~RunAction() = default;

void RunAction::BeginOfRunAction(const G4Run*) {
    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile("output.root");

    if (!fNtupleBooked) {
        analysisManager->CreateNtuple("nTuple", "nTuple");
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
        analysisManager->CreateNtupleIColumn("currentTrackID");
        analysisManager->CreateNtupleDColumn("EnergyInitial");
        analysisManager->CreateNtupleSColumn("PrimaryParticleName");
        analysisManager->CreateNtupleIColumn("PrimaryPDG");
        analysisManager->CreateNtupleDColumn("PrimaryEnergy");
        analysisManager->FinishNtuple(0);
        fNtupleBooked = true;
    }
}

void RunAction::EndOfRunAction(const G4Run*) {
    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}
