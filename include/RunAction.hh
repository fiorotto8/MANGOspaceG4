#ifndef RUNACTION_HH
#define RUNACTION_HH
#include "G4UserRunAction.hh"

#include <globals.hh>

class EventAction; // Forward declaration

class RunAction : public G4UserRunAction {
public:
    RunAction(EventAction* eventAction);
    ~RunAction() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

private:
    G4bool fNtupleBooked;
};
#endif
