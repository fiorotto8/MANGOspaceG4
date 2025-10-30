#ifndef B3StackingAction_h
#define B3StackingAction_h 1

#include "G4UserStackingAction.hh"
#include "globals.hh"

namespace B3 {

class StackingAction : public G4UserStackingAction {
public:
  StackingAction() = default;
  ~StackingAction() override = default;

  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track*) override;
};

} // namespace B3
#endif
