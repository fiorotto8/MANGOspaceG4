#include "DetectorConstruction.hh"

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4VisAttributes.hh>
#include <G4Box.hh>
#include <G4Orb.hh>
#include <G4SDManager.hh>
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include <G4MultiFunctionalDetector.hh>
#include <G4VPrimitiveScorer.hh>
#include <GasBoxSensitiveDetector.hh>
#include <G4Tubs.hh>

using namespace std;

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  //WORLD
  world_radius(1.5*m),//radius of world
  //Detector Box
  detWidth(12.*cm),//detectorBoxWidth
  detHeight(12.*cm),//detectorBoxHeight
  detThickness(12.*cm),//detectorBoxThickness
  openSideThickness(1*cm),// Thickness of the open side
  //GAS volume
  He_frac(0.6),
  CF4_frac(0.4),
  gasThickness(10.*cm),
  gasWidth(10.*cm),
  gasHeight(10.*cm)
  {
  // Any other initializations

}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4ThreeVector worldPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
  G4ThreeVector detectorPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
  G4ThreeVector gasPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
// Get nist material manager
    G4NistManager* nist = G4NistManager::Instance();

    // World
    //
    G4Material* world_mat = nist->FindOrBuildMaterial("G4_Galactic");

    // Option to switch on/off checking of volumes overlaps
    //
    G4bool checkOverlaps = false;

    // Parameters: inner radius, outer radius, start phi angle, delta phi angle, start theta angle, delta theta angle
    G4Sphere* solidWorld = new G4Sphere(
    "World",              // its name
    0.,                  // inner radius
    world_radius,        // outer radius
    0.,                  // start phi angle
    2 * CLHEP::pi,       // delta phi angle
    0.,                  // start theta angle
    CLHEP::pi);          // delta theta angle

  auto logicWorld = new G4LogicalVolume(solidWorld,  // its solid
    world_mat,                                       // its material
    "World");                                        // its name

  //Colorize the logic using proper vis. attributes
    auto logicWorldVis = new G4VisAttributes();
    logicWorldVis->SetVisibility(false);
    logicWorldVis->SetForceSolid(false);
    logicWorld->SetVisAttributes(logicWorldVis);

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
    worldPos,    // at (0,0,0)
    logicWorld,                                // its logical volume
    "World",                                   // its name
    nullptr,                                   // its mother  volume
    false,                                     // no boolean operation
    0,                                         // copy number
    checkOverlaps);                            // overlaps checking

  //
  // detector outer box
  //
    G4Material* Al = nist->FindOrBuildMaterial("G4_Al");

  // detector box shape
  auto solidFullBox = new G4Box("solidFullBox",                           // its name
    0.5 * detThickness, 0.5 * detWidth, 0.5 * detHeight);  // its size

  auto cutBox = new G4Box("CutBox", openSideThickness/2, 1.1*(detWidth/2), 1.1*(detHeight/2));

  G4SubtractionSolid* solidDetBox = new G4SubtractionSolid("solidDetBox", solidFullBox, cutBox, 0, G4ThreeVector(detThickness/2 - openSideThickness/2,0, 0));

  auto logicDetBox = new G4LogicalVolume(solidDetBox,  // its solid
    Al,                                         // its material
    "logicDetBox");                                  // its name


  //Colorize the logic using proper vis. attributes
    auto logicDetBoxVis = new G4VisAttributes(G4Colour::Red());
    logicDetBoxVis->SetVisibility(true);
    logicDetBoxVis->SetForceSolid(false);
    logicDetBox->SetVisAttributes(logicDetBoxVis);

  new G4PVPlacement(nullptr,  // no rotation
    detectorPos,                     // at position
    logicDetBox,              // its logical volume
    "detBox",                 // its name
    logicWorld,                 // its mother  volume
    false,                    // no boolean operation
    0,                        // copy number
    checkOverlaps);           // overlaps checking

  //////////////////////////////////////////////////////////////
  //
  // entering side
  //

  // Define the thickness of the aluminum plate
  G4double plateThickness = openSideThickness;  // Consistent with your other definitions
  // Define the circular hole diameter
  G4double holeDiameter = 10.5 * mm;  // Circular hole diameter
  // Define the plate geometry
  G4Box* solidPlate = new G4Box("solidPlate", 0.5 * detHeight, 0.5 * detWidth, 0.5 * plateThickness);
  // Define the hole geometry (cylindrical hole)
  G4Tubs* holeSolid = new G4Tubs("holeSolid", 0, 0.5 * holeDiameter,  plateThickness/2, 0, 360);
  // Define a rotation matrix to rotate the cylinder around the X-axis by 90 degrees
  G4RotationMatrix* rotMatrix = new G4RotationMatrix();
  rotMatrix->rotateY(90.0 * deg);  // Rotating by 90 degrees around the X-axis
  // Position of the hole (centered on the plate)
  G4ThreeVector holePos(0, 0, 0);
  // Subtract the hole from the plate to create a plate with a circular hole
  G4SubtractionSolid* plateWithHole = new G4SubtractionSolid("plateWithHole", solidPlate, holeSolid, 0, holePos);
  // Logical volume for the plate with a hole
  G4LogicalVolume* logicPlateWithHole = new G4LogicalVolume(plateWithHole, Al, "logicPlateWithHole");
  // Set visualization attributes
  G4VisAttributes* logicPlateWithHoleVis = new G4VisAttributes(G4Colour::Red());
  logicPlateWithHoleVis->SetVisibility(true);
  logicPlateWithHoleVis->SetForceSolid(false);
  logicPlateWithHole->SetVisAttributes(logicPlateWithHoleVis);
  // Calculate the position to attach the plate directly to the sensitive detector
  // Assuming the detector box is centered at origin and extends symmetrically
  G4ThreeVector platePosition(5*cm+.5*plateThickness, 0, 0);  // This positions the plate flush with the end of the detector box
  // Place the plate
  new G4PVPlacement(rotMatrix,                   // rotation
                    platePosition,             // at position
                    logicPlateWithHole,        // its logical volume
                    "plateWithHole",           // its name
                    logicWorld,                // its mother volume
                    false,                     // no boolean operation
                    0,                         // copy number
                    checkOverlaps);            // overlaps checking
/*
  //
  //Collimator
  //
  // Elements
  G4Element* Si = nist->FindOrBuildElement("Si");
  G4Element* O = nist->FindOrBuildElement("O");
  G4Element* Pb = nist->FindOrBuildElement("Pb");

  // Create Material
  G4double density_LG = 3.85 * g/cm3; // Typical density of lead glass
  G4Material* leadGlass = new G4Material("LeadGlass", density_LG, 3);
  leadGlass->AddElement(Si,  22 * perCent); // These are approximate values
  leadGlass->AddElement(O,  16 * perCent);
  leadGlass->AddElement(Pb,  62 * perCent);

  double activeDiameter=10*mm;
  double CollThickness = 10*mm;
  double poreDiameter= 500*um;
  //double poreDiameter= 6*um;//should be this
  double porePitch=1200*um;//i cannot go lower than that
  //double porePitch=8*um;//should be this
  // Calculate number of holes per row
  G4int numHolesPerRow = std::floor((activeDiameter - poreDiameter) / porePitch);
  G4double startX = -numHolesPerRow * porePitch / 2 + porePitch / 2;
  G4double startY = startX;
  G4Tubs* CollimatorCore = new G4Tubs("CollimatorCore", 0, 0.5 * holeDiameter,  CollThickness/2, 0, 360);
  //patterning
  // Use a G4SubtractionSolid only after you need to perform a subtraction
  G4Tubs* firsthole = new G4Tubs("hole", 0, poreDiameter/2, CollThickness, 0, 360 * deg);
  G4SubtractionSolid* currentSolid = new G4SubtractionSolid("currentSolid", CollimatorCore, firsthole, nullptr, G4ThreeVector(0,0,0));

  // Patterning the rest of the holes
  for (int i = 0; i < numHolesPerRow; ++i) {
    for (int j = 0; j < numHolesPerRow; ++j) {
      // Offset for honeycomb pattern
      G4double xPosition = startX + i * porePitch;
      G4double yPosition = startY + j * porePitch + (i % 2) * (porePitch / 2);

      // Check if the position is inside the disk
      if (std::sqrt(xPosition * xPosition + yPosition * yPosition) + poreDiameter / 2 <= 0.5 * activeDiameter && startX!=0 &&startY!=0) {
        G4ThreeVector holePos(xPosition, yPosition, 0);
        G4Tubs* hole = new G4Tubs("hole", 0, poreDiameter / 2, CollThickness, 0, 360 * deg);
        currentSolid = new G4SubtractionSolid("currentSolid", currentSolid, hole, nullptr, holePos);
      }
    }
  }


  // Create logical volume
  G4LogicalVolume* LogicCollWithPores = new G4LogicalVolume(currentSolid, leadGlass, "LogicDisk");
  // Set visualization attributes
  G4VisAttributes* LogicCollWithPoresVis = new G4VisAttributes(G4Colour::Green());
  LogicCollWithPoresVis->SetVisibility(true);
  LogicCollWithPoresVis->SetForceSolid(true);
  LogicCollWithPores->SetVisAttributes(LogicCollWithPoresVis);
  G4ThreeVector CollPosition(5*cm+.5*plateThickness, 0, 0);  // This positions the plate flush with the end of the detector box
    // Place the disk in the world volume
    new G4PVPlacement(rotMatrix,                    // no rotation
                      CollPosition,            // at (0,0,0)
                      LogicCollWithPores,                  // its logical volume
                      "Collimator",                     // its name
                      logicWorld,                 // its mother volume
                      false,                      // no boolean operation
                      0,                          // copy number
                      true);                      // checking overlaps
*/

  //
  // detector gas box
  //
  //GAS CREATION

    G4double massOfMole = 1.008*g/mole;
    G4double temperature = 293*kelvin;
    G4double pressure = 1*atmosphere; //this will be modified
    G4double density = 1000*g/m3; //this will be modified

    //He_gas
    G4Element* elHe = nist->FindOrBuildElement("He");
    density = 162.488*He_frac*g/m3;
    pressure = 1*He_frac*atmosphere;
    auto He_gas = new G4Material("He_gas", density, 1, kStateGas, temperature,pressure);
    He_gas->AddElement(elHe, 1);

    //Ar_gas
    G4Element* elAr = nist->FindOrBuildElement("Ar");
    density = 5.704*He_frac*kg/m3;
    pressure = 1*He_frac*atmosphere;
    auto Ar_gas = new G4Material("Ar_gas", density, 1, kStateGas, temperature,pressure);
    Ar_gas->AddElement(elAr, 1);

    //CF4_gas
    G4Element* elC  = nist->FindOrBuildElement("C");
    G4Element* elF = nist->FindOrBuildElement("F");
    density = 3574.736*CF4_frac*g/m3;
    pressure = 1*CF4_frac*atmosphere;
    auto CF4_gas = new G4Material("CF4_gas", density, 2, kStateGas, temperature, pressure);
    CF4_gas->AddElement(elC, 1);
    CF4_gas->AddElement(elF, 4);

    density = He_gas->GetDensity()+CF4_gas->GetDensity();
    //density = Ar_gas->GetDensity()+CF4_gas->GetDensity();
    pressure = He_gas->GetPressure()+CF4_gas->GetPressure();
    //pressure = Ar_gas->GetPressure()+CF4_gas->GetPressure();
    auto CYGNO_gas = new G4Material("CYGNO_gas", density, 2, kStateGas, temperature, pressure);
    CYGNO_gas->AddMaterial(He_gas, He_gas->GetDensity()/density*100*perCent);
    //CYGNO_gas->AddMaterial(Ar_gas, Ar_gas->GetDensity()/density*100*perCent);
    CYGNO_gas->AddMaterial(CF4_gas,  CF4_gas->GetDensity()/density*100*perCent);

  //
  // gas volume
  //
  auto solidGasBox = new G4Box("solidGasBox",                           // its name
   0.5 * gasThickness, 0.5 * gasWidth, 0.5 * gasHeight);  // its size

  auto logicGasBox = new G4LogicalVolume(solidGasBox,  // its solid
    CYGNO_gas,                                         // its material
    "logicGasBox");                                  // its name

  //Colorize the logic using proper vis. attributes
    auto logicGasBoxVis = new G4VisAttributes(G4Colour::Blue());
    logicGasBoxVis->SetVisibility(true);
    logicGasBoxVis->SetForceSolid(false);
    logicGasBox->SetVisAttributes(logicGasBoxVis);

  new G4PVPlacement(nullptr,  // no rotation
    gasPos,                     // at position
    logicGasBox,              // its logical volume
    "gasBox",                 // its name
    logicDetBox,                 // its mother  volume
    false,                    // no boolean operation
    0,                        // copy number
    checkOverlaps);           // overlaps checking

  // Create an instance of the sensitive detector
  GasBoxSensitiveDetector* gasBoxSD = new GasBoxSensitiveDetector("GasBoxSD");

  // Register the sensitive detector with the SD manager
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  SDManager->AddNewDetector(gasBoxSD);

  // Assign the sensitive detector to logicGasBox
  logicGasBox->SetSensitiveDetector(gasBoxSD);

  //
  //always return the physical World
  //
    return physWorld;
}

