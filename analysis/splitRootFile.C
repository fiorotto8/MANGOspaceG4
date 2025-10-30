void splitRootFile(const char* inputFileName,
                   const char* treeName = "nTuple")
{
    // open input
    TFile* inputFile = TFile::Open(inputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return;
    }

    // get tree
    TTree* inputTree = (TTree*)inputFile->Get(treeName);
    if (!inputTree) {
        std::cerr << "Error: Could not find tree " << treeName << " in file." << std::endl;
        inputFile->Close();
        delete inputFile;
        return;
    }

    Long64_t nEntries = inputTree->GetEntries();
    if (nEntries == 0) {
        std::cerr << "Warning: tree is empty." << std::endl;
        inputFile->Close();
        delete inputFile;
        return;
    }

    const int nFiles = 10;
    Long64_t entriesPerFile = nEntries / nFiles;   // integer division

    for (int i = 0; i < nFiles; ++i) {
        TString outputFileName = TString::Format("output_%d.root", i);
        TFile* outputFile = TFile::Open(outputFileName, "RECREATE");
        if (!outputFile || outputFile->IsZombie()) {
            std::cerr << "Error: Could not create output file " << outputFileName << std::endl;
            // don't forget to close input before returning
            inputFile->Close();
            delete inputFile;
            return;
        }

        outputFile->cd();  // make sure we are in the right file

        // clone structure only
        TTree* outputTree = inputTree->CloneTree(0);

        Long64_t startEntry = i * entriesPerFile;
        // last file takes the remainder
        Long64_t endEntry = (i == nFiles - 1) ? nEntries : (i + 1) * entriesPerFile;

        // edge case: if nEntries < nFiles, some files would have startEntry >= nEntries
        if (startEntry >= nEntries) {
            // write empty tree so files exist anyway
            outputTree->Write();
            outputFile->Close();
            delete outputFile;
            continue;
        }

        for (Long64_t j = startEntry; j < endEntry; ++j) {
            inputTree->GetEntry(j);
            outputTree->Fill();
        }

        outputTree->Write();
        outputFile->Close();
        delete outputFile;
    }

    inputFile->Close();
    delete inputFile;

    std::cout << "Splitting completed successfully!" << std::endl;
}
