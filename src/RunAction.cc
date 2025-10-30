// RunAction.cc
#include "RunAction.hh"
#include "EventAction.hh"
#include "G4Run.hh"
#include "G4Threading.hh"

// ROOT
#include "TFile.h"
#include "TTree.h"

namespace B3a {

void RunAction::BeginOfRunAction(const G4Run*)
{
  int tid = G4Threading::G4GetThreadId();  // -1 on master
  std::string fname = (tid < 0)
    ? "tpc_hits_master.root"
    : ("tpc_hits_t" + std::to_string(tid) + ".root");

  fOut  = TFile::Open(fname.c_str(), "RECREATE");
  fTree = new TTree("steps", "Ionizing hits in gas");

  // ints
  fTree->Branch("eventID",   &cols_.eventID);
  fTree->Branch("trackID",   &cols_.trackID);
  fTree->Branch("parentID",  &cols_.parentID);
  fTree->Branch("rootID",    &cols_.rootID);
  fTree->Branch("generation",&cols_.generation);
  fTree->Branch("pdg",       &cols_.pdg);
  fTree->Branch("creatorType",    &cols_.creatorType);
  fTree->Branch("creatorSubType", &cols_.creatorSubType);
  fTree->Branch("stepType",       &cols_.stepType);
  fTree->Branch("stepSubType",    &cols_.stepSubType);

  // NEW PE ints
  fTree->Branch("isPE",      &cols_.isPE);
  fTree->Branch("peTrackID", &cols_.peTrackID);
  fTree->Branch("nPEsec",    &cols_.nPEsec);

  // doubles
  fTree->Branch("x",&cols_.x);  fTree->Branch("y",&cols_.y);  fTree->Branch("z",&cols_.z);
  fTree->Branch("t",&cols_.t);
  fTree->Branch("px",&cols_.px); fTree->Branch("py",&cols_.py); fTree->Branch("pz",&cols_.pz);
  fTree->Branch("edep",&cols_.edep);
  fTree->Branch("stepLen",&cols_.stepLen);

  // NEW PE doubles
  fTree->Branch("pePx",   &cols_.pePx);
  fTree->Branch("pePy",   &cols_.pePy);
  fTree->Branch("pePz",   &cols_.pePz);
  fTree->Branch("peEkin", &cols_.peEkin);
  fTree->Branch("peTheta",&cols_.peTheta);
  fTree->Branch("pePhi",  &cols_.pePhi);
}

void RunAction::EndOfRunAction(const G4Run*)
{
  if (fOut) {
    fOut->Write();
    fOut->Close();
    fOut = nullptr;
    fTree = nullptr;
  }
}

void RunAction::FillFromSteps(const std::vector<EventAction::StepHit>& steps)
{
  cols_.clear();

  for (const auto& h : steps) {
    cols_.eventID.push_back(h.eventID);
    cols_.trackID.push_back(h.trackID);
    cols_.parentID.push_back(h.parentID);
    cols_.rootID.push_back(h.rootID);
    cols_.generation.push_back(h.generation);
    cols_.pdg.push_back(h.pdg);

    cols_.x.push_back(h.x);
    cols_.y.push_back(h.y);
    cols_.z.push_back(h.z);
    cols_.t.push_back(h.t);
    cols_.px.push_back(h.px);
    cols_.py.push_back(h.py);
    cols_.pz.push_back(h.pz);
    cols_.edep.push_back(h.edep);
    cols_.stepLen.push_back(h.stepLen);

    cols_.creatorType.push_back(h.creatorType);
    cols_.creatorSubType.push_back(h.creatorSubType);
    cols_.stepType.push_back(h.stepType);
    cols_.stepSubType.push_back(h.stepSubType);

    // NEW PE
    cols_.isPE.push_back(h.isPE);
    cols_.peTrackID.push_back(h.peTrackID);
    cols_.nPEsec.push_back(h.nPEsec);
    cols_.pePx.push_back(h.pePx);
    cols_.pePy.push_back(h.pePy);
    cols_.pePz.push_back(h.pePz);
    cols_.peEkin.push_back(h.peEkin);
    cols_.peTheta.push_back(h.peTheta);
    cols_.pePhi.push_back(h.pePhi);
  }

  if (fTree) fTree->Fill();
}

} // namespace B3a
