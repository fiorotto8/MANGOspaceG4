#include "GasBoxSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "EventAction.hh"

GasBoxSensitiveDetector::GasBoxSensitiveDetector(const G4String& name) :
G4VSensitiveDetector(name)
{}

GasBoxSensitiveDetector::~GasBoxSensitiveDetector()
{}

G4bool GasBoxSensitiveDetector::ProcessHits(G4Step * aStep, G4TouchableHistory* Rohist)
{

    G4Track* track = aStep->GetTrack();

    G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
    G4StepPoint* postStepPoint = aStep->GetPostStepPoint();

    G4ThreeVector posParticle = preStepPoint->GetPosition();

    G4String particleName = track->GetParticleDefinition()->GetParticleName();
    G4int particleID = track->GetTrackID();
    G4double EdepStep = aStep->GetTotalEnergyDeposit();
    G4int VolumeCopyNumber = track->GetVolume()->GetCopyNo();
    G4int particleParentID = track->GetParentID();
    G4ThreeVector TranslationVolVec = track->GetVolume()->GetTranslation(); 

    G4String ProcessType = "";

    if(track->GetCreatorProcess()){
    ProcessType= track->GetCreatorProcess()->GetProcessName();
    } 

    G4int particleTag=-1;

    if(particleName == "e-"){
    particleTag=0;
    } else if(particleName == "e+"){
    particleTag=1;
    } else if(particleName == "gamma"){
    particleTag=2;
    } else if(particleName == "alpha"){
    particleTag=3;
    } else {
    particleTag=-1;
    }
    // Retrieve initial kinetic energy of the particle
    G4double initialEnergy = track->GetVertexKineticEnergy();

    //G4cout << "position of: " << particleName <<" " << track->GetTrackID() << "  is:  "<< posParticle << " Energy deposited:  " << EdepStep << "  in volume:  " << VolumeCopyNumber << " ParentID: "  << track->GetParentID()<< " lastdecay: " << DecayElement << "  Process: " << ProcessType << G4endl;

    G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    // Get the current track ID
    G4int currentTrackID = track->GetTrackID();

    // Get the length of the step
    G4double stepLength = aStep->GetStepLength();
    // Get the PDG ID of the particle
    G4ParticleDefinition* particleDef = track->GetDefinition();
    G4int pdgID = particleDef->GetPDGEncoding();
    // Get the momentum direction of the particle
    G4ThreeVector momentumDirection = track->GetMomentumDirection();

    G4AnalysisManager* AnalysisManager = G4AnalysisManager::Instance(); 

    //TrackID is unique and counts the number of tracks generated in the simulation
    //ParentID is the ID of the parent track that generated the current track 
        //i.e. points directly to the track that generated the current track

    if (particleName == "e-"){

        AnalysisManager->FillNtupleIColumn(0,evt);
        AnalysisManager->FillNtupleSColumn(1,particleName);
        AnalysisManager->FillNtupleIColumn(2,particleID);
        AnalysisManager->FillNtupleIColumn(3,particleTag);
        AnalysisManager->FillNtupleIColumn(4,particleParentID);
        AnalysisManager->FillNtupleDColumn(5,posParticle[0]);
        AnalysisManager->FillNtupleDColumn(6,posParticle[1]);
        AnalysisManager->FillNtupleDColumn(7,posParticle[2]);
        AnalysisManager->FillNtupleDColumn(8,EdepStep);
        AnalysisManager->FillNtupleIColumn(9,VolumeCopyNumber);
        AnalysisManager->FillNtupleDColumn(10,TranslationVolVec[0]);
        AnalysisManager->FillNtupleDColumn(11,TranslationVolVec[1]);
        AnalysisManager->FillNtupleDColumn(12,TranslationVolVec[2]);
        AnalysisManager->FillNtupleSColumn(13,ProcessType);   
        AnalysisManager->FillNtupleDColumn(14,momentumDirection[0]);//px_particle
        AnalysisManager->FillNtupleDColumn(15,momentumDirection[1]);//py_particle
        AnalysisManager->FillNtupleDColumn(16,momentumDirection[2]);//pz_particle
        AnalysisManager->FillNtupleIColumn(17,pdgID);//pdg_id
        AnalysisManager->FillNtupleDColumn(18,stepLength);//lunghezza step
        AnalysisManager->FillNtupleIColumn(19,currentTrackID);//track ID for track distrimination
        AnalysisManager->FillNtupleDColumn(20,initialEnergy);//Initial energy of the particle

        AnalysisManager->AddNtupleRow(0);
    }
    return true;
}
