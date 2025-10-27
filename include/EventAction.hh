#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include <G4String.hh>
#include <G4ThreeVector.hh>
#include <G4UserEventAction.hh>

#include <vector>

class EventAction : public G4UserEventAction {
public:
    struct HitRecord {
        G4String        particleName;
        G4int           trackID;
        G4int           particleTag;
        G4int           parentID;
        G4ThreeVector   position;
        G4double        energyDeposit;
        G4int           volumeCopyNumber;
        G4ThreeVector   volumeTranslation;
        G4String        processType;
        G4ThreeVector   momentumDirection;
        G4int           pdgID;
        G4double        stepLength;
        G4double        initialEnergy;
    };

    EventAction();
    ~EventAction() override;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddHit(const HitRecord& record);

private:
    std::vector<HitRecord> fHitRecords;
    G4int                  fEventID;
    G4String               fPrimaryName;
    G4int                  fPrimaryPDG;
    G4double               fPrimaryEnergy;
};

#endif
