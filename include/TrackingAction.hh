/// \file TrackingAction.hh
/// \brief Definition of the TrackingAction class

#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class EventAction;
class DetectorConstruction;
class TrackingMessenger;

class TrackingAction : public G4UserTrackingAction {

public:  
    TrackingAction(EventAction*,DetectorConstruction*);
    ~TrackingAction();

    virtual void  PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);

    void SetFullChain(G4bool flag) { fFullChain = flag;};
    void SetTimeWindow(G4double , G4double);

    private:
    EventAction*        fEvent;
    DetectorConstruction* fDetector;
    G4double fCharge, fMass;        
    G4bool   fFullChain;



    G4double fTime_birth,  fTime_end;
    G4double fTimeWindow1, fTimeWindow2;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
