#ifndef RUNACTION_HH
#define RUNACTION_HH
#include "G4UserRunAction.hh"
#include "TFile.h"
#include "TTree.h"

class EventAction; // Forward declaration

class RunAction : public G4UserRunAction {
public:
    RunAction(EventAction* eventAction);
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;

private:
    EventAction* eventAction;
    TFile* outputFile;
    TTree* tree;
};
#endif
