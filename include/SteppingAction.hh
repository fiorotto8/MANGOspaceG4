#pragma once
#include "G4UserSteppingAction.hh"

namespace B3a {

class EventAction;

class SteppingAction : public G4UserSteppingAction {
public:
  explicit SteppingAction(EventAction* ea);
  void UserSteppingAction(const G4Step* step) override;

private:
  EventAction* fEventAction; // not owned
};

} // namespace B3a
