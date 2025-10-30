#include <TFile.h>
#include <TTree.h>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>

// ---------------------
// Geometry/constants
// ---------------------
static const double InnerSourceContThick = 5;     // mm
static const double GasRadius            = 36.9;  // mm
static const double GasDistanceFromCollim= 10;    // mm
static const double CollimatorDepth      = 2;     // mm
static const double CollimatorDistance   = 0;     // mm
static const double GasThickness         = 50;    // mm
static const double containment_off      = 5;     // mm

// angular window around -z (±30°)
static const double acceptedAngleStart = M_PI - (30.*M_PI/180.);
static const double acceptedAngleEnd   = M_PI + (30.*M_PI/180.);

// derived
static const double cyl_center_z =
    InnerSourceContThick/2. + CollimatorDepth + CollimatorDistance
  + GasRadius + GasDistanceFromCollim;

// ---------------------
// Helpers
// ---------------------
inline bool IsPointAcceptable(double x, double z) {
  const double dx = x;
  const double dz = z - cyl_center_z;
  const double maxR  = GasRadius - containment_off;
  const double maxR2 = maxR * maxR;
  const double r2    = dx*dx + dz*dz;
  double angle = std::atan2(dx, dz); // [-pi,pi]
  if (angle < 0) angle += 2.*M_PI;   // [0,2pi]
  const bool inAngle = (angle >= acceptedAngleStart && angle <= acceptedAngleEnd);
  return (r2 <= maxR2) || inAngle;
}

static std::string PDGName(int pdg){
  switch(pdg){
    case 11:  return "e-";
    case -11: return "e+";
    case 22:  return "gamma";
    case 2212:return "proton";
    case 2112:return "neutron";
    default:  return std::to_string(pdg);
  }
}



// ---------------------
// Main
// ---------------------
void RecoTrack_faster(const std::string& filename="tpc_hits.root")
{
  const double W_factor = 38e-6; // MeV per ion pair

  TFile* f = TFile::Open(filename.c_str());
  if(!f || f->IsZombie()){ std::cerr<<"Cannot open "<<filename<<"\n"; return; }

  TTree* tree = (TTree*)f->Get("steps");
  if(!tree){ std::cerr<<"Tree 'steps' not found\n"; return; }

  // vector branches per event
  std::vector<int>    *eventID=nullptr,*trackID=nullptr,*parentID=nullptr,*rootID=nullptr,*generation=nullptr,*pdg=nullptr;
  std::vector<int>    *creatorType=nullptr,*creatorSubType=nullptr,*stepType=nullptr,*stepSubType=nullptr;
  std::vector<double> *x=nullptr,*y=nullptr,*z=nullptr,*t=nullptr,*px=nullptr,*py=nullptr,*pz=nullptr,*edep=nullptr,*stepLen=nullptr;

  tree->SetBranchAddress("eventID", &eventID);
  tree->SetBranchAddress("trackID", &trackID);
  tree->SetBranchAddress("parentID",&parentID);
  tree->SetBranchAddress("rootID",  &rootID);
  tree->SetBranchAddress("generation",&generation);
  tree->SetBranchAddress("pdg",     &pdg);
  tree->SetBranchAddress("x",&x);   tree->SetBranchAddress("y",&y);   tree->SetBranchAddress("z",&z);
  tree->SetBranchAddress("t",&t);
  tree->SetBranchAddress("px",&px); tree->SetBranchAddress("py",&py); tree->SetBranchAddress("pz",&pz);
  tree->SetBranchAddress("edep",&edep);
  tree->SetBranchAddress("stepLen",&stepLen);
  tree->SetBranchAddress("creatorType",&creatorType);
  tree->SetBranchAddress("creatorSubType",&creatorSubType);
  tree->SetBranchAddress("stepType",&stepType);
  tree->SetBranchAddress("stepSubType",&stepSubType);

  // output: one row per (event, rootID) cluster
  std::string outName = "elab_" + filename;
  TFile* fout = TFile::Open(outName.c_str(), "RECREATE");
  TTree* out  = new TTree("elabHits","per-primary clusters from gas hits");

  int    Out_event=-1, Out_rootID=-1, Out_nhits=0;
  double Out_ETotal=0.0, Out_TrackLength=0.0, Out_Primaries=0.0;
  bool   Out_FullyContained=true;
  std::string Out_ParticleLabel;

  std::vector<double> X, Y, Z, T, Edep, PX, PY, PZ;

  out->Branch("EventNumber",    &Out_event);
  out->Branch("RootID",         &Out_rootID);
  out->Branch("nhits",          &Out_nhits);
  out->Branch("TotalEDep",      &Out_ETotal);
  out->Branch("TrackLength",    &Out_TrackLength);
  out->Branch("Primaries",      &Out_Primaries);
  out->Branch("FullyContained", &Out_FullyContained);
  out->Branch("ParticleLabel",  &Out_ParticleLabel);
  out->Branch("x_hits",   &X);
  out->Branch("y_hits",   &Y);
  out->Branch("z_hits",   &Z);
  out->Branch("t_hits",   &T);
  out->Branch("Edep_hits",&Edep);
  out->Branch("px_hits",  &PX);
  out->Branch("py_hits",  &PY);
  out->Branch("pz_hits",  &PZ);

  const Long64_t nEv = tree->GetEntries();
  for(Long64_t ie=0; ie<nEv; ++ie){
    tree->GetEntry(ie);
    if(!x || x->empty()) continue;

    // group hits by primary ancestor
    std::map<int, std::vector<size_t>> byRoot;
    byRoot.clear();
    for(size_t i=0;i<x->size();++i) byRoot[ (*rootID)[i] ].push_back(i);

    for(auto& kv : byRoot){
      const int rid = kv.first;
      auto& idx = kv.second;

      // sort by time to compute path length
      std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return (*t)[a] < (*t)[b]; });

      // reset outputs
      X.clear(); Y.clear(); Z.clear(); T.clear();
      Edep.clear(); PX.clear(); PY.clear(); PZ.clear();
      Out_event = (*eventID)[0];
      Out_rootID = rid;
      Out_ETotal = 0.0;
      Out_TrackLength = 0.0;
      Out_FullyContained = true;

      std::map<int,int> pdgCount;

      X.reserve(idx.size()); Y.reserve(idx.size()); Z.reserve(idx.size());
      T.reserve(idx.size()); Edep.reserve(idx.size());
      PX.reserve(idx.size()); PY.reserve(idx.size()); PZ.reserve(idx.size());

      for(size_t k=0;k<idx.size();++k){
        size_t i = idx[k];
        X.push_back((*x)[i]);  Y.push_back((*y)[i]);  Z.push_back((*z)[i]);
        T.push_back((*t)[i]);
        Edep.push_back((*edep)[i]);
        PX.push_back((*px)[i]); PY.push_back((*py)[i]); PZ.push_back((*pz)[i]);

        Out_ETotal += (*edep)[i];
        pdgCount[ (*pdg)[i] ]++;

        if(k>0){
          const double dx = (*x)[i]-(*x)[ idx[k-1] ];
          const double dy = (*y)[i]-(*y)[ idx[k-1] ];
          const double dz = (*z)[i]-(*z)[ idx[k-1] ];
          Out_TrackLength += std::sqrt(dx*dx + dy*dy + dz*dz);
        }
        if(Out_FullyContained && !IsPointAcceptable((*x)[i], (*z)[i])) Out_FullyContained = false;
      }

      Out_nhits = (int)idx.size();
      Out_Primaries = Out_ETotal / W_factor;

      // dominant PDG in the cluster
      int bestPDG=0, bestN=-1;
      for(auto& pp : pdgCount){ if(pp.second>bestN){ bestN=pp.second; bestPDG=pp.first; } }
      Out_ParticleLabel = PDGName(bestPDG);

      out->Fill();
    }
  }

  fout->Write();
  fout->Close();
  f->Close();

  std::cout << "Wrote clusters to " << outName << "\n";
}
