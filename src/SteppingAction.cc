#include "SteppingAction.hh"
#include "EventAction.hh"

#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include <cstdio>
#include <cmath>

namespace B3a {

namespace {
  inline bool IsGasLV(const G4Step* s) {
    const auto* pre = s->GetPreStepPoint();
    if (!pre) return false;
    const auto  touch = pre->GetTouchableHandle();
    if (!touch) return false;
    const auto* vol = touch->GetVolume();
    if (!vol) return false;
    const auto* lv  = vol->GetLogicalVolume();
    return lv && lv->GetName() == "TPCGasLV";
  }
}

SteppingAction::SteppingAction(EventAction* ea) : fEventAction(ea) {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!IsGasLV(step)) return;

  const auto edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.) return;

  auto* rm   = G4RunManager::GetRunManager();
  auto* trk  = step->GetTrack();
  auto* pre  = step->GetPreStepPoint();
  auto* post = step->GetPostStepPoint();

  EventAction::StepHit h{};

  // ----- standard fill -----
  h.eventID  = rm->GetCurrentEvent()->GetEventID();
  h.trackID  = trk->GetTrackID();
  h.parentID = trk->GetParentID();
  h.pdg      = trk->GetDefinition()->GetPDGEncoding();

  // ancestry
  auto& primMap = fEventAction->primaryOfTrack();
  auto& genMap  = fEventAction->generationOfTrack();
  if (primMap.find(h.trackID) == primMap.end()) {
    if (h.parentID == 0) {
      primMap[h.trackID] = h.trackID;
      genMap[h.trackID]  = 0;
    } else {
      auto itP = primMap.find(h.parentID);
      primMap[h.trackID] = (itP != primMap.end()) ? itP->second : h.parentID;
      auto itG = genMap.find(h.parentID);
      genMap[h.trackID]  = (itG != genMap.end()) ? (itG->second + 1) : 1;
    }
  }
  h.rootID     = primMap[h.trackID];
  h.generation = genMap[h.trackID];

  const auto pos = pre->GetPosition();
  h.x = pos.x()/mm; h.y = pos.y()/mm; h.z = pos.z()/mm;
  h.t = pre->GetGlobalTime()/ns;

  const auto mom = pre->GetMomentum();
  h.px = mom.x(); h.py = mom.y(); h.pz = mom.z();

  h.edep    = edep/MeV;
  h.stepLen = step->GetStepLength()/mm;

  const auto* cp = trk->GetCreatorProcess();
  h.creatorType    = cp ? cp->GetProcessType()    : -1;
  h.creatorSubType = cp ? cp->GetProcessSubType() : -1;

  const auto* sp = post ? post->GetProcessDefinedStep() : nullptr;
  h.stepType      = sp ? sp->GetProcessType()    : -1;
  h.stepSubType   = sp ? sp->GetProcessSubType() : -1;

  // ---- init PE fields (default = no PE) ----
  h.isPE      = 0;
  h.peTrackID = -1;
  h.pePx = h.pePy = h.pePz = 0.0;
  h.peEkin  = 0.0;
  h.peTheta = 0.0;
  h.pePhi   = 0.0;
  h.nPEsec  = 0;

    // ======================================================
    // STRICT primary-gamma photoelectric capture
    // ======================================================
    if (sp &&
        sp->GetProcessType() == 2 &&          // EM
        sp->GetProcessSubType() == 12)        // photoelectric
    {
    const bool isPrimaryGamma = (h.pdg == 22) && (h.parentID == 0);
    if (isPrimaryGamma) {

        const auto& secs = *(step->GetSecondaryInCurrentStep());

        // 1) try to find the electron CREATED BY THIS VERY PROCESS (sp)
        const G4VProcess* thisPEproc = sp;
        const G4Track* chosenEle = nullptr;
        int nEle = 0;

        // also keep a fallback = highest-KE e-
        const G4Track* fallbackEle = nullptr;
        double fallbackKE = -1.0; // MeV

        for (const auto* s : secs) {
        if (!s) continue;
        const auto* def = s->GetDefinition();
        if (!def) continue;
        if (def->GetPDGEncoding() != 11) continue; // only electrons
        ++nEle;

        // is this electron produced by the SAME process as the step?
        const auto* sProc = s->GetCreatorProcess();
        if (sProc == thisPEproc) {
            // this is the real photoelectron we wanted
            chosenEle = s;
            break;  // we can stop
        }

        // otherwise, update fallback (highest KE)
        const double ke = s->GetKineticEnergy() / MeV;
        if (ke > fallbackKE) {
            fallbackKE  = ke;
            fallbackEle = s;
        }
        }

        // 2) decide which one to use
        const G4Track* pe = chosenEle ? chosenEle : fallbackEle;

        if (pe) {
        const auto eMom = pe->GetMomentum();   // MeV/c
        const double epx = eMom.x();
        const double epy = eMom.y();
        const double epz = eMom.z();
        const double ekin = pe->GetKineticEnergy() / MeV;

        // angles
        double theta = 0.0, phi = 0.0;
        const double p2 = epx*epx + epy*epy + epz*epz;
        const double p  = std::sqrt(p2);
        if (p > 0.) {
            theta = std::acos(epz / p);
            phi   = std::atan2(epy, epx);
            if (phi < 0.) phi += 2.0*M_PI;
        }

        h.isPE      = 1;
        h.peTrackID = pe->GetTrackID();
        h.pePx      = epx;
        h.pePy      = epy;
        h.pePz      = epz;
        h.peEkin    = ekin;
        h.peTheta   = theta;
        h.pePhi     = phi;
        h.nPEsec    = nEle;   // total electrons we saw in this PE step
        }
    }
    }


  // store
  fEventAction->steps().push_back(h);
  fEventAction->AddToTotalEdepGas(h.edep);
}

} // namespace B3a
