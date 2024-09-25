// Function to compute the azimuth angle (theta)
double computeAzimuth(double swappedX, double y) {
    return atan2(y, swappedX);  // Returns azimuth angle in radians
}

// Function to compute the inclination angle (phi)
double computeInclination(double swappedZ, double y, double swappedX) {
    double r = sqrt(swappedX * swappedX + y * y + swappedZ * swappedZ); // Magnitude of the vector
    return acos(swappedZ / r);  // Returns inclination angle in radians
}

// Calculate the z-coordinate of the cylinder's center
double GasRadius = 36.9; //mm
double xedge_min= -25.0;//mm    
double xedge_max= 25.0;//mm
double containmentOff = 5.0;//mm

bool areAllPointsInsideCylinder(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& z) {
    // Iterate over all points
    for (size_t i = 0; i < x.size(); ++i) {
        // Calculate the distance from the center of the cylinder (assumed to be at (0, cylCenterZ))
        double dy = y[i]; // radial distance in the x-y plane
        double dz = z[i];
        double r = std::sqrt(dy * dy + dz * dz); // radial distance in the x-z plane

        // Check if the point is inside the circle or within the accepted angle
        if (!(r <= (GasRadius - containmentOff) && x[i] >= xedge_min + containmentOff && x[i] <= xedge_max - containmentOff)) {
            // Optionally, for debugging, print which point is outside the acceptable conditions
            // std::cout << "Point at index " << i << " is outside the acceptable area" << std::endl;
            return false;  // If any point is outside the acceptable conditions, return false immediately
        }
    }
    return true;  // If all points are inside the acceptable conditions, return true
}

// Function to process track data from a ROOT file and write selected information to a new ROOT file.
void RecoTrack(std::string filename) {
    // Open the input ROOT file.
    TFile* f = TFile::Open(filename.c_str());
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    
    // Retrieve a TTree named "nTuple" from the file.
    TTree* tree = (TTree*)f->Get("nTuple");
    if (!tree) {
        std::cerr << "Error: Cannot find TTree 'nTuple' in file " << filename << std::endl;
        f->Close();
        return;
    }

    // Define variables to hold data from the tree.
    Int_t Evn, ParticleID, ParticleTag, ParentID, VolumeNumber;
    Double_t px_hits, py_hits,pz_hits,x_hits, y_hits, z_hits, VolumeTraslX, VolumeTraslY, VolumeTraslZ, EnergyDeposit, EnergyInitial,stepLength;
    Char_t ParticleName[20];
    Char_t CreationProcess[20];

    // Set the addresses of local variables where tree data will be stored during the reading process.
    tree->SetBranchAddress("EventNumber", &Evn);
    tree->SetBranchAddress("ParticleID", &ParticleID);
    tree->SetBranchAddress("ParticleTag", &ParticleTag);
    tree->SetBranchAddress("ParticleName", &ParticleName);
    tree->SetBranchAddress("ParentID", &ParentID);
    tree->SetBranchAddress("VolumeNumber", &VolumeNumber);
    tree->SetBranchAddress("x_hits", &x_hits);
    tree->SetBranchAddress("y_hits", &y_hits);
    tree->SetBranchAddress("z_hits", &z_hits);
    tree->SetBranchAddress("px_particle", &px_hits);
    tree->SetBranchAddress("py_particle", &py_hits);
    tree->SetBranchAddress("pz_particle", &pz_hits);
    tree->SetBranchAddress("EnergyDeposit", &EnergyDeposit);
    tree->SetBranchAddress("VolumeTraslX", &VolumeTraslX);
    tree->SetBranchAddress("VolumeTraslY", &VolumeTraslY);
    tree->SetBranchAddress("VolumeTraslZ", &VolumeTraslZ);
    tree->SetBranchAddress("ProcessType", &CreationProcess);
    tree->SetBranchAddress("EnergyInitial", &EnergyInitial);
    tree->SetBranchAddress("tracklen_hits", &stepLength);

    // Initialize variables for output data.
    Int_t Out_event,partID;
    Double_t ETotal, EInit, length, startposX, startposY, startposZ, startPX, startPY, startPZ, startAzimut, startIncl;
    Int_t nhits_out;
    std::vector<Double_t> px_hits_out,py_hits_out,pz_hits_out, x_hits_out, y_hits_out, z_hits_out, EdepHits_out;
    std::string partName, CreationProc;
    bool fullyCont;

    // Create a new ROOT file to store output data.
    TFile* f_out = new TFile(Form("elab_%s", filename.c_str()), "recreate");
    TTree* outTree = new TTree("elabHits", "elabHits");

    // Define branches for the output tree.
    outTree->Branch("EventNumber", &Out_event);
    outTree->Branch("ParticleID", &partID);
    outTree->Branch("CreatingProcess", &CreationProc);
    outTree->Branch("EDep", &ETotal);
    outTree->Branch("EInit", &EInit);
    outTree->Branch("nhits", &nhits_out);
    //outTree->Branch("x_hits", &x_hits_out);
    //outTree->Branch("y_hits", &y_hits_out);
    //outTree->Branch("z_hits", &z_hits_out);
    //outTree->Branch("Edep_hits", &EdepHits_out);
    outTree->Branch("FullyContained", &fullyCont);
    outTree->Branch("ParticleName", &partName);
    outTree->Branch("travelLength", &length);
    outTree->Branch("StartPositionX", &startposX);
    outTree->Branch("StartPositionY", &startposY);
    outTree->Branch("StartPositionZ", &startposZ);
    outTree->Branch("StartMomentumX", &startPX);
    outTree->Branch("StartMomentumY", &startPY);
    outTree->Branch("StartMomentumZ", &startPZ);
    outTree->Branch("StartAzimuth", &startAzimut);
    outTree->Branch("StartInclination", &startIncl);

    // Read the first entry to initialize variables.
    tree->GetEntry(0);
    Out_event = Evn;
    CreationProc = CreationProcess;
    partID = ParticleID;
    ETotal = 0;
    nhits_out = 1;
    fullyCont = true;
    EInit = EnergyInitial; // Initialize EInit for the first particle
    partName = ParticleName; // Initialize partName for the first particle
    startposX=x_hits;
    startposY=y_hits;
    startposZ=z_hits;
    startPX=px_hits;
    startPY=py_hits;
    startPZ=pz_hits;
    startAzimut = computeAzimuth(startPZ, startPY);
    startIncl = computeInclination(startPZ, startPY, startPX);
    length=0;

    // Process each entry in the tree.
    for (int i = 0; i < tree->GetEntries(); i++) {
        // Print progress for every 10000 entries processed.
        if (i % 10000 == 0) std::cout << i << "/" << tree->GetEntries() << std::endl;

        // Read the current entry.
        tree->GetEntry(i);

        //cout<<Out_event<<" "<<Evn<<" "<<partID<<" "<<ParticleID<<endl;
        // Check if the current hit belongs to the same event and nucleus as previously processed.
        if (Out_event == Evn && partID == ParticleID) {
            // Accumulate total energy deposited.
            ETotal += EnergyDeposit;
            // Store hit data.
            x_hits_out.push_back(x_hits);
            y_hits_out.push_back(y_hits);
            z_hits_out.push_back(z_hits);
            px_hits_out.push_back(px_hits);
            py_hits_out.push_back(py_hits);
            pz_hits_out.push_back(pz_hits);
            EdepHits_out.push_back(EnergyDeposit);
            nhits_out++;
            length+=stepLength;
        }
        else {
            //cout<<Evn<<" "<<ParticleName<<" "<<EnergyInitial<<" "<<Nucleus<<" "<<CreationProcess<<endl;

            // If a new event or nucleus is encountered, save the data from the previous event.
            fullyCont = areAllPointsInsideCylinder(x_hits_out, y_hits_out, z_hits_out);
            outTree->Fill();

            // Reset variables for the new event.
            Out_event = Evn;
            partID = ParticleID;
            EInit = 0;
            partName = ParticleName;
            ETotal = 0;
            length=0;
            nhits_out = 1;
            x_hits_out.clear();
            y_hits_out.clear();
            z_hits_out.clear();
            px_hits_out.clear();
            py_hits_out.clear();
            pz_hits_out.clear();
            EdepHits_out.clear();

            // Start accumulating new event data.
            ETotal += EnergyDeposit;
            x_hits_out.push_back(x_hits);
            y_hits_out.push_back(y_hits);
            z_hits_out.push_back(z_hits);
            EdepHits_out.push_back(EnergyDeposit);
            fullyCont = true;
            length=stepLength;
            startposX=x_hits;
            startposY=y_hits;
            startposZ=z_hits;
            startPX=px_hits;
            startPY=py_hits;
            startPZ=pz_hits;
            startAzimut = computeAzimuth(startPZ, startPY);
            startIncl = computeInclination(startPZ, startPY, startPX);            EInit = EnergyInitial; // Initialize EInit for the first particle
            CreationProc = CreationProcess;
        } // end of if-else
    } // end of for loop

    // Write the remaining data to the file.
    f_out->cd();
    outTree->Write();
    f_out->Save();
    f_out->Close();
}