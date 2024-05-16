// GasBoxSensitiveDetector.hh

#ifndef GasBoxSensitiveDetector_h
#define GasBoxSensitiveDetector_h

#include "G4VSensitiveDetector.hh"
#include <map>
#include <vector>
#include <string>
#include <G4String.hh>

class GasBoxSensitiveDetector : public G4VSensitiveDetector {
public:
    GasBoxSensitiveDetector(const G4String& name);
    virtual ~GasBoxSensitiveDetector();

    // Mandatory methods
    virtual void Initialize(G4HCofThisEvent* HCE) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* ROhist) override;
    virtual void EndOfEvent(G4HCofThisEvent* HCE) override;
    // Methods to a access primaries info
    G4bool HasParticleArrived() const { return BoolArrived; }
    G4bool HasParticleInteracted() const { return BoolInteracted; }
    G4double GetTotPrimEnergyDeposit() const { return totalPrimaryEnergyDeposit; }
    G4double GetPrimaryEnergy() const { return primaryEnergy; }
    G4int GetPDGPrimary() const { return primaryPDG; }
    G4int GetPrimaryHits() const { return primaryHits; }
    //methods to access secondary info
    G4int GetSecondaryHits() const { return secondaryHits; }

    // Reset method for the event
    void ResetForNewEvent();
    // Getters for the secondary count and types

private:
//Primaries
    G4bool BoolArrived;
    G4bool BoolInteracted;
    G4double totalPrimaryEnergyDeposit;
    G4double primaryEnergy;
    G4int primaryPDG;
    G4int primaryHits;
//Secondaries
    G4int secondaryHits;
};

#endif // GasBoxSensitiveDetector_h

