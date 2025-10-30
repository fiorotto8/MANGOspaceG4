// EventAction.hh
#pragma once
#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector>
#include <unordered_map>

class G4Event;

namespace B3a {

class RunAction;

class EventAction : public G4UserEventAction {
public:
  struct StepHit {
    // existing fields
    G4int    eventID, trackID, parentID, rootID, generation, pdg;
    G4double x, y, z, t;     // mm, ns
    G4double px, py, pz;     // MeV/c
    G4double edep;           // MeV
    G4double stepLen;        // mm
    // process meta
    G4int    creatorType;     // G4ProcessType enum value
    G4int    creatorSubType;  // process-specific subtype
    G4int    stepType;        // process that defined the step: type
    G4int    stepSubType;     // process that defined the step: subtype

    // -------------------------------------------------
    // NEW: photoelectric-specific info (per step)
    // -------------------------------------------------
    G4int    isPE;            // 1 if this step was photoelectric, else 0
    G4int    peTrackID;       // trackID of emitted e-
    G4double pePx, pePy, pePz; // e- momentum (MeV/c)
    G4double peEkin;          // e- kinetic energy (MeV)
    G4double peTheta;         // polar angle of e- (rad)
    G4double pePhi;           // azimuthal angle of e- (rad)
    G4int    nPEsec;          // how many e- secondaries found
  };

  G4double totalEdepGas() const { return fTotalEdepGas; }
  void AddToTotalEdepGas(G4double dE) { fTotalEdepGas += dE; }

  explicit EventAction(RunAction* runAction);
  ~EventAction() override = default;

  void BeginOfEventAction(const G4Event*) override;
  void EndOfEventAction  (const G4Event*) override;

  std::vector<StepHit>&       steps()       { return fSteps; }
  const std::vector<StepHit>& steps() const { return fSteps; }

  std::unordered_map<int,int>& primaryOfTrack()    { return fPrimaryOfTrack; }
  std::unordered_map<int,int>& generationOfTrack() { return fGenerationOfTrack; }

  void Clear() {
    fSteps.clear();
    fPrimaryOfTrack.clear();
    fGenerationOfTrack.clear();
    fTotalEdepGas = 0.0;
  }

private:
  RunAction* fRunAction = nullptr;

  std::vector<StepHit> fSteps;
  std::unordered_map<int,int> fPrimaryOfTrack;     // trackID -> root primary trackID
  std::unordered_map<int,int> fGenerationOfTrack;  // trackID -> 0,1,2,...
  G4double fTotalEdepGas = 0.0;
};

} // namespace B3a
