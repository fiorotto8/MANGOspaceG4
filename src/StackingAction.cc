/// \file B3/B3a/src/StackingAction.cc
#include "StackingAction.hh"

#include "G4Track.hh"
#include "G4NeutrinoE.hh"

namespace B3 {

G4ClassificationOfNewTrack
StackingAction::ClassifyNewTrack(const G4Track* track)
{
  // Keep primaries and all secondaries by default
  // Kill neutrinos to save time
  if (track->GetDefinition() == G4NeutrinoE::NeutrinoE()) return fKill;

  return fUrgent;
}

} // namespace B3
