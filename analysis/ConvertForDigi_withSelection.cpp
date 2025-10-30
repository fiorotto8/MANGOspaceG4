#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

// =========================
// Geometry / global constants (same as your original)
// =========================
double InnerSourceContThick = 5;
double GasRadius            = 36.9; // mm
double GasDistanceFromCollim= 10;
double CollimatorDepth      = 2;
double CollimatorDistance   = 0;
double GasThickness         = 50;
double containment_off      = 5; // mm

// Angular acceptance around -z
double acceptedAngleStart = M_PI - (30*M_PI/180.0);
double acceptedAngleEnd   = M_PI + (30*M_PI/180.0);

// Derived geometry
double cyl_center_z = InnerSourceContThick / 2
                    + CollimatorDepth
                    + CollimatorDistance
                    + GasRadius
                    + GasDistanceFromCollim;

double zedge_min = InnerSourceContThick / 2
                 + CollimatorDepth
                 + CollimatorDistance
                 + GasDistanceFromCollim;

double zedge_max = InnerSourceContThick / 2
                 + CollimatorDepth
                 + CollimatorDistance
                 + 2*GasRadius
                 + GasDistanceFromCollim;

// =========================
// Containment check (same logic as your original)
// =========================
bool areAllPointsInsideCylinder(const std::vector<double>& x,
                                const std::vector<double>& y,
                                const std::vector<double>& z) {
    for (size_t i = 0; i < x.size(); ++i) {
        double dx = x[i];
        double dz = z[i] - cyl_center_z;
        double r2 = dx*dx + dz*dz;

        double angle = atan2(dx, dz);
        if (angle < 0) angle += 2.0 * M_PI;

        bool inAngle = (angle >= acceptedAngleStart && angle <= acceptedAngleEnd);
        const double maxR  = GasRadius - containment_off;
        const double maxR2 = maxR*maxR;

        if (!(r2 <= maxR2 || inAngle)) return false;
    }
    return true;
}

// =========================
// MAIN
// =========================
int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_file.root> <output_basename> <fill_option: 1=check, 0=fill_all>\n";
        return 1;
    }

    const std::string input_filename  = argv[1];
    const std::string output_basename = argv[2];
    const bool check_points = (std::stoi(argv[3]) == 1);

    // --- Open input and fetch 'steps'
    TFile *file = TFile::Open(input_filename.c_str());
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file " << input_filename << "\n";
        return 1;
    }
    TTree *steps = nullptr;
    file->GetObject("steps", steps);
    if (!steps) {
        std::cerr << "Error: TTree 'steps' not found in " << input_filename << "\n";
        return 1;
    }

    // --- Branch pointers (vectorized per event)
    std::vector<int>    *eventID=nullptr,*trackID=nullptr,*parentID=nullptr,*rootID=nullptr,*generation=nullptr,*pdg=nullptr;
    std::vector<int>    *creatorType=nullptr,*creatorSubType=nullptr,*stepType=nullptr,*stepSubType=nullptr;
    std::vector<double> *x=nullptr,*y=nullptr,*z=nullptr,*t=nullptr,*px=nullptr,*py=nullptr,*pz=nullptr,*edep=nullptr,*stepLen=nullptr;

    steps->SetBranchAddress("eventID", &eventID);
    steps->SetBranchAddress("trackID", &trackID);
    steps->SetBranchAddress("parentID",&parentID);
    steps->SetBranchAddress("rootID",  &rootID);
    steps->SetBranchAddress("generation",&generation);
    steps->SetBranchAddress("pdg",     &pdg);

    steps->SetBranchAddress("x",&x);    steps->SetBranchAddress("y",&y);    steps->SetBranchAddress("z",&z);
    steps->SetBranchAddress("t",&t);
    steps->SetBranchAddress("px",&px);  steps->SetBranchAddress("py",&py);  steps->SetBranchAddress("pz",&pz);
    steps->SetBranchAddress("edep",&edep);
    steps->SetBranchAddress("stepLen",&stepLen);

    steps->SetBranchAddress("creatorType", &creatorType);
    steps->SetBranchAddress("creatorSubType",&creatorSubType);
    steps->SetBranchAddress("stepType",    &stepType);
    steps->SetBranchAddress("stepSubType", &stepSubType);

    // --- Output file / tree (identical structure & names to your original)
    TFile* f_out = new TFile((output_basename + ".root").c_str(), "RECREATE");
    TTree* outTree = new TTree("nTuple", "nTuple");

    Int_t    Out_event      = -1;
    Int_t    nhits_out      = 0;
    Double_t ETotal         = 0.0;  // keV
    Double_t ETotal_NR      = 0.0;  // keV (not available → set 0)

    std::vector<Int_t>    pdgID;
    std::vector<Double_t> tracklen;       // per-step length (mm)
    std::vector<Double_t> px_part, py_part, pz_part; // MeV/c per hit
    std::vector<Double_t> EdepHits_out;   // per-hit edep (keV)
    std::vector<Double_t> x_hits_out, y_hits_out, z_hits_out; // mm

    outTree->Branch("eventnumber",     &Out_event);
    outTree->Branch("numhits",         &nhits_out);
    outTree->Branch("energyDep",       &ETotal);
    outTree->Branch("energyDep_NR",    &ETotal_NR);
    outTree->Branch("pdgID_hits",      &pdgID);
    outTree->Branch("tracklen_hits",   &tracklen);
    outTree->Branch("px_particle",     &px_part);
    outTree->Branch("py_particle",     &py_part);
    outTree->Branch("pz_particle",     &pz_part);
    outTree->Branch("energyDep_hits",  &EdepHits_out);
    outTree->Branch("x_hits",          &x_hits_out);
    outTree->Branch("y_hits",          &y_hits_out);
    outTree->Branch("z_hits",          &z_hits_out);

    const double MeV_to_keV = 1000.0;
    Long64_t nEvents = steps->GetEntries();

    int nFilled = 0;
    for (Long64_t ie = 0; ie < nEvents; ++ie) {
        steps->GetEntry(ie);
        if (!x || x->empty()) continue; // no gas hits in this event

        // Group hits in this event by primary ancestor
        std::map<int, std::vector<size_t>> byRoot;
        for (size_t i = 0; i < x->size(); ++i) {
            byRoot[ (*rootID)[i] ].push_back(i);
        }

        // Build one output row per (event, rootID) "cluster"
        for (auto& kv : byRoot) {
            const int rid = kv.first;
            auto& idx = kv.second;

            // Sort indices by time to keep trajectories ordered (optional but useful)
            std::sort(idx.begin(), idx.end(),
                      [&](size_t a, size_t b){ return (*t)[a] < (*t)[b]; });

            // Reset per-cluster outputs
            Out_event = (*eventID)[0];
            nhits_out = (Int_t)idx.size();
            ETotal    = 0.0;
            ETotal_NR = 0.0;

            pdgID.clear();
            tracklen.clear();
            px_part.clear(); py_part.clear(); pz_part.clear();
            EdepHits_out.clear();
            x_hits_out.clear(); y_hits_out.clear(); z_hits_out.clear();

            pdgID.reserve(idx.size());
            tracklen.reserve(idx.size());
            px_part.reserve(idx.size()); py_part.reserve(idx.size()); pz_part.reserve(idx.size());
            EdepHits_out.reserve(idx.size());
            x_hits_out.reserve(idx.size()); y_hits_out.reserve(idx.size()); z_hits_out.reserve(idx.size());

            for (size_t k = 0; k < idx.size(); ++k) {
                const size_t i = idx[k];

                // Positions (mm)
                x_hits_out.push_back( (*x)[i] );
                y_hits_out.push_back( (*y)[i] );
                z_hits_out.push_back( (*z)[i] );

                // Momentum (MeV/c)
                px_part.push_back( (*px)[i] );
                py_part.push_back( (*py)[i] );
                pz_part.push_back( (*pz)[i] );

                // PDG
                pdgID.push_back( (*pdg)[i] );

                // Per-step length (mm)
                tracklen.push_back( (*stepLen)[i] );

                // Energy deposit per hit (keV) and total (keV)
                const double e_keV = (*edep)[i] * MeV_to_keV;
                EdepHits_out.push_back(e_keV);
                ETotal += e_keV;
            }

            // Optional containment veto (same behavior as your original flag)
            if (!check_points || areAllPointsInsideCylinder(x_hits_out, y_hits_out, z_hits_out)) {
                outTree->Fill();
                ++nFilled;
            }
        }
    }

    outTree->Write();
    f_out->Close();
    file->Close();
    std::cout << "Wrote " << nFilled << " clusters to " << (output_basename + ".root") << std::endl;
    return 0;
}
