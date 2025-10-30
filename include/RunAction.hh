// RunAction.hh
#pragma once
#include <vector>
#include <string>
#include "G4UserRunAction.hh"
#include "EventAction.hh"  // we need the struct

class TFile;
class TTree;

namespace B3a {

struct StepFlatColumns {
  // ints
  std::vector<int> eventID, trackID, parentID, rootID, generation, pdg;
  std::vector<int> creatorType, creatorSubType, stepType, stepSubType;

  // NEW ints for PE
  std::vector<int> isPE;
  std::vector<int> peTrackID;
  std::vector<int> nPEsec;

  // doubles
  std::vector<double> x, y, z, t, px, py, pz, edep, stepLen;

  // NEW doubles for PE
  std::vector<double> pePx, pePy, pePz;
  std::vector<double> peEkin;
  std::vector<double> peTheta;
  std::vector<double> pePhi;

  void clear() {
    eventID.clear(); trackID.clear(); parentID.clear(); rootID.clear(); generation.clear(); pdg.clear();
    creatorType.clear(); creatorSubType.clear(); stepType.clear(); stepSubType.clear();
    isPE.clear(); peTrackID.clear(); nPEsec.clear();
    x.clear(); y.clear(); z.clear(); t.clear(); px.clear(); py.clear(); pz.clear(); edep.clear(); stepLen.clear();
    pePx.clear(); pePy.clear(); pePz.clear();
    peEkin.clear(); peTheta.clear(); pePhi.clear();
  }
};


class RunAction : public G4UserRunAction {
public:
  RunAction() = default;
  ~RunAction() override = default;

  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction  (const G4Run*) override;

  void FillFromSteps(const std::vector<EventAction::StepHit>& steps);

private:
  TFile* fOut  = nullptr;
  TTree* fTree = nullptr;
  StepFlatColumns cols_;
};

} // namespace B3a
