#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"

namespace B3a {

EventAction::EventAction(RunAction* runAction)
: G4UserEventAction(), fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*) { Clear(); }

void EventAction::EndOfEventAction(const G4Event*)
{
  fRunAction->FillFromSteps(fSteps);
}

} // namespace B3a
