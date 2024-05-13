// GasBoxSensitiveDetector.cc
#include "GasBoxSensitiveDetector.hh"

GasBoxSensitiveDetector::GasBoxSensitiveDetector(const G4String& name)
    : G4VSensitiveDetector(name) {
    // Constructor implementation
}

GasBoxSensitiveDetector::~GasBoxSensitiveDetector() {
    // Destructor implementation
}

void GasBoxSensitiveDetector::Initialize(G4HCofThisEvent* HCE) {
    ResetForNewEvent();
}

// ProcessHits function in GasBoxSensitiveDetector class
// This function is called by Geant4 when a particle interacts with the sensitive detector
G4bool GasBoxSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* ROhist) {
    // Get the track of the particle involved in this step of the simulation
    G4Track* track = step->GetTrack();
    // Obtain the pre-step point
    G4StepPoint* preStepPoint = step->GetPreStepPoint();

    // From the track you can obtain the pointer to the dynamic particle:
    const G4DynamicParticle* dynParticle = track -> GetDynamicParticle();

    // The dynamic particle class contains e.g. the kinetic energy after the step:
    // The pre-step point contains the kinetic energy at the beginning of the step
    G4double kinEnergy = preStepPoint->GetKineticEnergy();
    G4double PDGcode = dynParticle -> GetPDGcode();
    G4ThreeVector momentumDir = dynParticle -> GetMomentumDirection();

    G4int particleID=track->GetParentID();

    if (particleID == 0) {// This is a primary particle in the sensitive volume
        BoolArrived = true;
        //get PDGcode
        primaryPDG=PDGcode;
        //Get its energy
        primaryEnergy = kinEnergy;
        // Get the energy deposited in this step
        G4double stepEnergyDeposit = step->GetTotalEnergyDeposit();
        // Accumulate the energy deposited by the primary particle
        // Assuming totalPrimaryEnergyDeposit is a member variable of your sensitive detector class
        totalPrimaryEnergyDeposit += stepEnergyDeposit;
        if (totalPrimaryEnergyDeposit>0){BoolInteracted=true;}

    } else if(particleID==1){// This is a secondary particle from the primary particle
        secondariesPDG.push_back(PDGcode);
        numSecondaries++;
    } else{// A secondary with secondary parent (empty for now)

    }


    // Return true to indicate successful processing of the hit
    return true;
}

void GasBoxSensitiveDetector::EndOfEvent(G4HCofThisEvent* HCE) {
    G4cout << "Primary Energy(keV): " << primaryEnergy*1000. << G4endl;
    G4cout << "Particle arrived? " << BoolArrived << G4endl;
    G4cout << "Particle Interacted? " << BoolInteracted << G4endl;
    G4cout << "Particle deposit (keV): " << totalPrimaryEnergyDeposit*1000. << G4endl;
    G4cout << "Number of Secondaries: " << numSecondaries << G4endl;
    for (const auto& entry : secondariesPDG) {
        G4cout << "Secondary particle: " << entry << G4endl;
    }
}

void GasBoxSensitiveDetector::ResetForNewEvent() {
    //primaries
    BoolArrived = false;
    BoolInteracted = false;
    totalPrimaryEnergyDeposit=0;
    primaryEnergy=0;
    primaryPDG=0;
    //secondaries
    numSecondaries=0;
    secondariesPDG.clear();
}