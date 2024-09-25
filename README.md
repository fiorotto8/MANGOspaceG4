# MANGO in Space simulation

## GEANT

### Geometry

- For now just a 10cm side cube of gas surrounded by 1cm of Aluminum
- The entrance side has a 1.05cm diameter hole for entrance
- In the hole is supposed to be placed a MCP plate <https://tectra.de/wp-content/uploads/2017/07/MCP.pdf>
  - Geometry is implemented but commented since it crashed the graphics
- Getting primary energy of gamma and deposited energy in the gas, also other stuff but this is needed for efficiency
- Using `myMac.mac`

- cut on the particles is in GasBoxSensitiveDetector (i.e. saving only e-)
- cut of contained events is in ConvertFrDigi ( not yet implemtned)
- containment boolean in Reco.c is wrong

## ANALYSIS

- root -l 'RecoTrack.C("output_t0.root")'
  - root elab_output_t0.root
  - TTree* tree = (TTree*)_file0->Get("elabHits");
  - tree->Draw("EDep>>histName(250, 0.001, 0.06)", "(EDep < 0.06 && EDep > 0.001) && (ParticleName == \"e-\" || ParticleName == \"gamma\")")

OR

- g++ -o convert ConvertForDigi.cpp `root-config --cflags --libs`

## old analysis

- in analysis/ folder `efficiency.py`
- condition for *seen photon*: `(df["BoolInteracted"]==1) & (df["primaryHits"]==1 ) & (df["energyDeposit"]>df["primaryEnergy"]-epsilon)& (df["energyDeposit"]<=df["primaryEnergy"])`
  - espsilon usually 200eV
