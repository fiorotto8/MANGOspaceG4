#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
// ... other includes ...

void ActionInitialization::Build() const {
    SetUserAction(new PrimaryGeneratorAction());

    // Create an instance of EventAction
    EventAction* eventAction = new EventAction();

    // Create an instance of RunAction and pass eventAction to it
    RunAction* runAction = new RunAction(eventAction);
    SetUserAction(runAction);

    // Set the eventAction
    SetUserAction(eventAction);

    // ... other action initializations ...
}

void ActionInitialization::BuildForMaster() const {
    // For master thread in multi-threaded mode, we don't need the EventAction
    // Just create the RunAction without passing EventAction
    SetUserAction(new RunAction(nullptr));
}