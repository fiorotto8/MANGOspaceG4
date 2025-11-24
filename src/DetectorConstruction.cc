// DetectorConstruction.cc
#include "DetectorConstruction.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"
#include "G4Tubs.hh"

namespace B3 {

DetectorConstruction::DetectorConstruction() {  }

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // ✅ define nist here
  auto* nist = G4NistManager::Instance();

  //! World (vacuum-like)
  auto* worldMat   = nist->FindOrBuildMaterial("G4_Galactic");
  auto* solidWorld = new G4Box("World", 50*cm, 50*cm, 50*cm);
  auto* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
  auto* physWorld  = new G4PVPlacement(
      nullptr, {}, logicWorld, "World", nullptr, false, 0, fCheckOverlaps);

  //! Gas (active TPC volume)
  //defining detector gas mixture
  //

  G4double aHe = 4.002602*g/mole;
  G4Element* elHe = new G4Element("Helium","He", 2, aHe);

  //density 1394
  G4double aAr = 39.948*g/mole;
  G4Element* elAr = new G4Element("Argon","Ar", 18, aAr);

  G4double aC = 12.0107*g/mole;
  G4Element* elC = new G4Element("Carbon", "C", 6, aC);
  
  G4double aF=18.998*g/mole;
  G4Element* elF = new G4Element("Fluorine"  ,"F" , 9., aF);

  G4double aS =87.62*g/mole;
  G4Element* elS = new G4Element("Sulfur","S",16,aS);

  // User knobs
  G4double gasPressure    = 1 * atmosphere;  // <-- you change only this
  G4double gasTemperature = 300*kelvin;

  // Fractions (sum to 1.0 in volume/partial pressure sense)
  G4double He_frac  = 0.6;
  G4double CF4_frac = 0.4;
  G4double Ar_frac  = 0.;
  G4double SF6_frac = 0.;

  // Reference densities at 1 atm, 300 K for *pure* gas
  // (your numbers were these *times* the fraction already; we separate it)
  G4double rhoHe_ref   = 162.488  * g/m3;   // He @1 atm
  G4double rhoCF4_ref  = 3574.736 * g/m3;   // CF4 @1 atm
  G4double rhoAr_ref   = 1394.0   * g/m3;   // Ar @1 atm
  G4double rhoSF6_ref  = 6010.368 * g/m3;   // SF6 @1 atm

  // Scale by total pressure and by fraction
  // ρ_component = ρ_ref * (gasPressure / 1 atm) * fraction
  G4double densityHe   = rhoHe_ref  * (gasPressure/atmosphere) * He_frac;
  G4double densityCF4  = rhoCF4_ref * (gasPressure/atmosphere) * CF4_frac;
  G4double densityAr   = rhoAr_ref  * (gasPressure/atmosphere) * Ar_frac;
  G4double densitySF6  = rhoSF6_ref * (gasPressure/atmosphere) * SF6_frac;

  // Partial pressures (still useful to store)
  G4double pressureHe   = gasPressure * He_frac;
  G4double pressureCF4  = gasPressure * CF4_frac;
  G4double pressureAr   = gasPressure * Ar_frac;
  G4double pressureSF6  = gasPressure * SF6_frac;

  // Build each component material
  auto* He_gas = new G4Material("He_gas",  densityHe, 1,
                                kStateGas, gasTemperature, pressureHe);
  He_gas->AddElement(elHe,1);

  auto* CF4_gas = new G4Material("CF4_gas", densityCF4, 2,
                                kStateGas, gasTemperature, pressureCF4);
  CF4_gas->AddElement(elC,1);
  CF4_gas->AddElement(elF,4);

  auto* Ar_gas = new G4Material("Ar_gas", densityAr, 1,
                                kStateGas, gasTemperature, pressureAr);
  Ar_gas->AddElement(elAr,1);

  auto* SF6_gas = new G4Material("SF6_gas", densitySF6, 2,
                                kStateGas, gasTemperature, pressureSF6);
  SF6_gas->AddElement(elS,1);
  SF6_gas->AddElement(elF,6);

  // Now the mixture
  G4double densityMix   = densityHe + densityCF4 + densityAr + densitySF6;
  G4double pressureMix  = pressureHe + pressureCF4 + pressureAr + pressureSF6;

  auto* CYGNO_gas = new G4Material("CYGNO_gas", densityMix, 4,
                                  kStateGas, gasTemperature, pressureMix);

  // Add components by MASS FRACTION = componentMass / totalMass
  CYGNO_gas->AddMaterial(He_gas,  densityHe  / densityMix);
  CYGNO_gas->AddMaterial(CF4_gas, densityCF4 / densityMix);
  CYGNO_gas->AddMaterial(Ar_gas,  densityAr  / densityMix);
  CYGNO_gas->AddMaterial(SF6_gas, densitySF6 / densityMix);

  //! Creating the TPC gas volume
  //

  G4double GasRadius = 36.9 * mm;
  G4double GasThickness = 50 * mm;

  auto* solidGas = new G4Tubs("TPCGas", 0, GasRadius, GasThickness / 2, 0, 360 * deg);
  auto* logicGas = new G4LogicalVolume(solidGas, CYGNO_gas, "TPCGasLV");

  // place cylinder so that one endcap (flat face) is centered at the origin (z=0)
  G4ThreeVector gasPos(0., 0., GasThickness / 2.); // center shifted +dz so lower face sits at z=0
  new G4PVPlacement(nullptr, gasPos, logicGas, "TPCGas",
                    logicWorld, false, 0, fCheckOverlaps);

  // Optional visibility
  logicGas->SetVisAttributes(new G4VisAttributes(G4Colour(0.68, 0.85, 0.9))); // Light blue and fully colored

  // Region-specific cuts for the TPC gas (fine, low-energy secondaries)
  auto* gasRegion = new G4Region("TPCGasRegion");
  auto* gasCuts   = new G4ProductionCuts();
  gasCuts->SetProductionCut(0.001*mm); // ~1 µm — aggressive for gas
  gasRegion->SetProductionCuts(gasCuts);

  // Attach region to the gas logical volume
  logicGas->SetRegion(gasRegion);
  gasRegion->AddRootLogicalVolume(logicGas);

  // ✅ must return the world physical volume
  return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
  auto* gas = new G4MultiFunctionalDetector("gas");
  G4SDManager::GetSDMpointer()->AddNewDetector(gas);
  gas->RegisterPrimitive(new G4PSEnergyDeposit("edep"));

  // Attach by logical volume name (as in B3a pattern)
  SetSensitiveDetector("TPCGasLV", gas);
}

} // namespace B3
