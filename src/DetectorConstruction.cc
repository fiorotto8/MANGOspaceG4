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
#include <G4PhysicalVolumeStore.hh>

using namespace std;

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  //WORLD
  world_radius(1.5*m),//radius of world
  //!GAS CREATION
  He_frac(0.6),
  CF4_frac(0.4),
  Ar_frac(0.),
  gasThickness(10.*cm),
  gasWidth(10.*cm),
  gasHeight(10.*cm),
  gasPressure(1*atmosphere),
  //! plexiglass box
  plexiWidth(15.*cm),
  plexiHeight(15.*cm),
  plexiThickness(7.*cm),
  plexiSideThickness(1*cm),
  windowThickness(0.1*mm),
  //! Field Cage
  radialRingThickness(1.0 * cm),
  RingThicknessAlongDrift(0.5 * cm),  
  GasRadius(36.9 * mm),
  GasThickness(50 * mm),
  trenchHeight(1 * mm),
  trenchMinRadius(GasRadius + 1 * mm), 
  Nrings(6),
  ringSpacing(10. * mm),
  //! SOURCE
  outerSourceRad(25./2*mm),
  innerSourceRad(10./2*mm),
  InnerSourceContThick(5.*mm),
  sourceDepth(0.01*mm),
  collimatorRadius(16. / 2 *mm),
  collimatorDepth(2.0*mm),
  collimatorHoleRadius(2. / 2 *mm),
  collimatorDistance(0.0*mm),
  additionalDistance(1.*mm),
  //OLD
  //! detector Alu outer box 
  detWidth(12.*cm),//detectorBoxWidth
  detHeight(12.*cm),//detectorBoxHeight
  detThickness(12.*cm),//detectorBoxThickness
  openSideThickness(1*cm)// Thickness of the open side

  {
  // Any other initializations

}

DetectorConstruction::~DetectorConstruction() {
    // Destructor implementation (can be empty)
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{
  //NISt manager
  G4NistManager* nist = G4NistManager::Instance();


//! WORLD
  G4ThreeVector worldPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
  G4ThreeVector detectorPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
  G4ThreeVector gasPos = G4ThreeVector(0.*cm, 0.*cm, 0.*cm);
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

//!GAS CREATION

  G4double massOfMole = 1.008*g/mole;
  G4double temperature = 293*kelvin;
  G4double pressure = 1*atmosphere; //this will be modified
  G4double density = 1000*g/m3; //this will be modified

  //He_gas
  G4Element* elHe = nist->FindOrBuildElement("He");
  auto He_gas = new G4Material("He_gas", 162.488*He_frac*g/m3, 1, kStateGas, temperature,gasPressure*He_frac);
  He_gas->AddElement(elHe, 1);

  //Ar_gas
  G4Element* elAr = nist->FindOrBuildElement("Ar");
  auto Ar_gas = new G4Material("Ar_gas", 5704*Ar_frac*g/m3, 1, kStateGas, temperature,gasPressure*Ar_frac);
  Ar_gas->AddElement(elAr, 1);

  //CF4_gas
  G4Element* elC  = nist->FindOrBuildElement("C");
  G4Element* elF = nist->FindOrBuildElement("F");
  auto CF4_gas = new G4Material("CF4_gas", 3574.736*CF4_frac*g/m3, 2, kStateGas, temperature, gasPressure*CF4_frac);
  CF4_gas->AddElement(elC, 1);
  CF4_gas->AddElement(elF, 4);

  //initializing the gas mixture
  auto CYGNO_gas = new G4Material("CYGNO_gas", density, 2, kStateGas, temperature, pressure);

  if (Ar_frac==0){
    //HeCF4
    density = He_gas->GetDensity()+CF4_gas->GetDensity();
    pressure = He_gas->GetPressure()+CF4_gas->GetPressure();
    CYGNO_gas->AddMaterial(He_gas, He_gas->GetDensity()/density*100*perCent);
    CYGNO_gas->AddMaterial(CF4_gas,  CF4_gas->GetDensity()/density*100*perCent);
  }
  else{
    //ArCF4
    density = Ar_gas->GetDensity()+CF4_gas->GetDensity();
    pressure = Ar_gas->GetPressure()+CF4_gas->GetPressure();
    CYGNO_gas->AddMaterial(Ar_gas, Ar_gas->GetDensity()/density*100*perCent);
    CYGNO_gas->AddMaterial(CF4_gas,  CF4_gas->GetDensity()/density*100*perCent);
  }

//! plexiglass box
  // Define elements: Carbon, Hydrogen, Oxygen
  //G4Element* elC = nist->FindOrBuildElement("C");
  G4Element* elH = nist->FindOrBuildElement("H");
  G4Element* elO = nist->FindOrBuildElement("O");

  // Define density of Plexiglass
  G4double densityPlexi = 1.19 * g/cm3;

  // Create a new material called "Plexiglass"
  G4Material* plexiglass = new G4Material("Plexiglass", densityPlexi, 3);  // 3 components
  plexiglass->AddElement(elC, 5);
  plexiglass->AddElement(elH, 8);
  plexiglass->AddElement(elO, 2);

  // Define the outer full box (outer dimensions of Plexiglass box)
  G4Box* outerBox = new G4Box("OuterBox", 0.5 * plexiThickness, 0.5 * plexiWidth, 0.5 * plexiHeight);

  // Define the larger inner box to hollow out the core and leave two open sides
  // The thickness of this inner box is larger than the outer box in the X direction (to ensure both sides are open),
  // but the width and height are smaller to leave the side walls intact.
  G4double innerThickness = plexiThickness *1.2;  // Make the thickness larger to ensure the front and back sides are open
  G4double innerWidth = plexiWidth - 2 * plexiSideThickness;  // Leave side walls intact
  G4double innerHeight = plexiHeight - 2 * plexiSideThickness;  // Leave top and bottom intact
  G4Box* innerBox = new G4Box("InnerBox", 0.5 * innerThickness, 0.5 * innerWidth, 0.5 * innerHeight);

  // Subtract innerBox from outerBox to create the hollow Plexiglass box with two open sides
  G4SubtractionSolid* hollowBox = new G4SubtractionSolid("HollowBox", outerBox, innerBox);

  // Create the logical volume for the Plexiglass box
  G4LogicalVolume* logicPlexiBox = new G4LogicalVolume(hollowBox, plexiglass, "logicPlexiBox");

  // Visualization attributes for the Plexiglass box (semi-transparent)
  G4VisAttributes* logicPlexiBoxVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.5));  // White with 50% transparency
  logicPlexiBoxVis->SetVisibility(true);
  logicPlexiBoxVis->SetForceSolid(true);
  logicPlexiBox->SetVisAttributes(logicPlexiBoxVis);

  // Place the Plexiglass box in the world volume
  new G4PVPlacement(nullptr, detectorPos, logicPlexiBox, "detBox", logicWorld, false, 0, checkOverlaps);       // overlaps checking   

  // Define Mylar material (PET) using NIST database
  G4Material* mylar = nist->FindOrBuildMaterial("G4_MYLAR");

  // Define the Mylar window size, using the same y and z dimensions as the Plexiglass box
  G4Box* mylarWindow = new G4Box("MylarWindow", 0.5 * windowThickness, 0.5 * plexiWidth, 0.5 * plexiHeight);

//! Mylar window
  // Create the logical volume for the Mylar window
  G4LogicalVolume* logicMylarWindow = new G4LogicalVolume(mylarWindow, mylar, "logicMylarWindow");

  // Position the Mylar window at the front of the Plexiglass box
  // Assuming the Plexiglass box is placed at "detectorPos", we offset the Mylar window slightly in X direction
  G4ThreeVector mylarWindowPos(detectorPos.x() - 0.5 * plexiThickness - 0.5 * windowThickness, detectorPos.y(), detectorPos.z());

  // Place the Mylar window in the world volume
  new G4PVPlacement(nullptr,                       // No rotation
      mylarWindowPos,                              // Position in front of Plexiglass
      logicMylarWindow,                            // Logical volume of the Mylar window
      "MylarWindow",                               // Name
      logicWorld,                                  // Mother volume (world)
      false,                                       // No boolean operation
      0,                                           // Copy number
      checkOverlaps);                              // Check for overlaps

  // Visualization attributes for the Mylar window (semi-transparent)
  G4VisAttributes* logicMylarWindowVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0., 0.3));  // White with 50% transparency
  logicMylarWindowVis->SetVisibility(true);
  logicMylarWindowVis->SetForceSolid(true);
  logicMylarWindow->SetVisAttributes(logicMylarWindowVis);

//! Additional window

  // Define Black Tape material (using polyethylene as an approximation)
  G4Material* blackTape = nist->FindOrBuildMaterial("G4_POLYETHYLENE");  // Approximation for electrician's tape

  // Define the thickness of the Black Tape window
  G4double blackTapeThickness = 0.15 * mm;  // You can adjust this thickness as per your requirements

  // Define the Black Tape window size, using the same y and z dimensions as the Plexiglass box
  G4Box* blackTapeWindow = new G4Box("BlackTapeWindow", 0.5 * blackTapeThickness, 0.5 * plexiWidth, 0.5 * plexiHeight);

  // Create the logical volume for the Black Tape window
  G4LogicalVolume* logicBlackTapeWindow = new G4LogicalVolume(blackTapeWindow, blackTape, "logicBlackTapeWindow");

  // Set the visualization attributes: Black color (representing black tape)
  G4VisAttributes* blackTapeWindowVis = new G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 0.5));  // Black color (RGB: 0.0, 0.0, 0.0) fully opaque
  blackTapeWindowVis->SetVisibility(true);
  blackTapeWindowVis->SetForceSolid(true);  // Make sure it's drawn as solid
  logicBlackTapeWindow->SetVisAttributes(blackTapeWindowVis);

  // Position the Black Tape window slightly further away from the Mylar window
  // Let's say we place it 1 mm away from the Mylar window along the X-axis
  G4ThreeVector blackTapeWindowPos(mylarWindowPos.x() - 10 * mm - 0.5 * blackTapeThickness, mylarWindowPos.y(), mylarWindowPos.z());

  // Place the Black Tape window in the world volume
  new G4PVPlacement(nullptr,                       // No rotation
      blackTapeWindowPos,                          // Position near Mylar window
      logicBlackTapeWindow,                        // Logical volume of the Black Tape window
      "BlackTapeWindow",                           // Name
      logicWorld,                                  // Mother volume (world)
      false,                                       // No boolean operation
      0,                                           // Copy number
      checkOverlaps);                              // Check for overlaps

//! gas volume

  G4Box* gasRegion = new G4Box("GasRegion", 
                              0.5 * (plexiThickness ),  // Thickness of hollow region
                              0.5 * (plexiWidth - 2 * plexiSideThickness),      // Width of hollow region
                              0.5 * (plexiHeight - 2 * plexiSideThickness));    // Height of hollow region

  G4LogicalVolume* logicGasRegion = new G4LogicalVolume(gasRegion, CYGNO_gas, "logicGasRegion");

  // Assuming the hollow Plexiglass is centered, the gas region can be placed at the same position
  new G4PVPlacement(nullptr,                         // No rotation
                    G4ThreeVector(0, 0, 0),          // Positioned at the center of the Plexiglass box
                    logicGasRegion,                  // Logical volume for the gas
                    "GasRegion",                     // Name of the gas volume
                    logicPlexiBox,                   // Parent volume (Plexiglass box)
                    false,                           // No boolean operations
                    0,                               // Copy number
                    checkOverlaps);                  // Overlap check
  
  G4VisAttributes* logicGasRegionVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.05));  // 
  logicGasRegionVis->SetVisibility(true);
  logicGasRegionVis->SetForceSolid(true);  // Make sure it's drawn as solid
  logicGasRegion->SetVisAttributes(logicGasRegionVis);

//! Field Cage
  // Define Silver material and PLA material
  G4Material* Silver = nist->FindOrBuildMaterial("G4_Ag");

  G4double densityPLA = 1.25 * g/cm3;  // Typical density of PLA
  G4Material* PLA = new G4Material("PLA", densityPLA, 3);  // 3 components: C, H, O
  PLA->AddElement(elC, 3);  // 3 Carbon atoms
  PLA->AddElement(elH, 4);  // 4 Hydrogen atoms
  PLA->AddElement(elO, 2);  // 2 Oxygen atoms

  // Define visualization attributes for the rings
  G4Colour darkGreyColor(0.3, 0.3, 0.3, 1.0);  // Opaque dark grey
  G4VisAttributes* RingVisAttributes = new G4VisAttributes(darkGreyColor);
  RingVisAttributes->SetForceSolid(true);

  // Define visualization attributes for the Silver fill
  G4Colour lightGreyColor(0.7, 0.7, 0.7, 1.0);  // Opaque light grey
  G4VisAttributes* FieldRingAttributes = new G4VisAttributes(lightGreyColor);
  FieldRingAttributes->SetForceSolid(true);

  // Create solid geometry for the ring
  G4Tubs* solidRing = new G4Tubs("solidRing", GasRadius, GasRadius + radialRingThickness, RingThicknessAlongDrift / 2, 0, 360);

  // Create solid geometry for the trench
  G4Tubs* solidTrench = new G4Tubs("solidTrench", GasRadius - 10 * mm, trenchMinRadius, trenchHeight / 2, 0, 360);

  // Combine the ring and the trench
  G4VSolid* solidWithTrench = new G4SubtractionSolid("solidWithTrench", solidRing, solidTrench, 0, G4ThreeVector(0, 0, 0));

  // Create logical volume for the ring with trench
  G4LogicalVolume* logicRingWithTrench = new G4LogicalVolume(solidWithTrench, PLA, "RingWithTrench");

  // Set visualization attributes for the ring with trench
  logicRingWithTrench->SetVisAttributes(RingVisAttributes);

  // Create solid geometry for Silver filling the trench
  G4Tubs* solidSilverFill = new G4Tubs("solidSilverFill", GasRadius, trenchMinRadius, trenchHeight / 2, 0, 360);

  // Create logical volume for Silver filling
  G4LogicalVolume* logicSilverFill = new G4LogicalVolume(solidSilverFill, Silver, "SilverFill");
  logicSilverFill->SetVisAttributes(FieldRingAttributes);

  // Place the Silver fill in the trench
  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicSilverFill, "SilverFill", logicRingWithTrench, false, 0);

  // Rotation matrix for the ring
  G4RotationMatrix* rotY = new G4RotationMatrix();
  rotY->rotateY(90 * degree);

  // Loop to place the rings
  for (int i = 0; i < Nrings; i++) {
      G4double xPos = (i - 2.5) * ringSpacing;  // Center between the 3rd and 4th rings, ensuring symmetry
      
      // Place the ring along the X-axis, with the middle space between rings centered at the origin
      new G4PVPlacement(rotY,                      // No rotation needed (rings parallel to YZ plane)
                        G4ThreeVector(xPos, 0, 0),    // Place along the X-axis
                        logicRingWithTrench,          // Logical volume of the ring with trench
                        "RingWithTrench",             // Name
                        logicWorld,                   // Mother volume (world)
                        false,                        // No boolean operations
                        i);                           // Copy number (use i as the copy number)
  }


//! Cathode

G4double xPosNegativeRing = (-2.5) * ringSpacing;

// Step 1: Define Kapton and Copper materials using NIST
G4Material* Kapton = nist->FindOrBuildMaterial("G4_KAPTON");
G4Material* Copper = nist->FindOrBuildMaterial("G4_Cu");

// Step 2: Define the thickness of the windows
G4double kaptonThickness = 45 * micrometer;  // 45 microns for Kapton
G4double copperThickness = 35 * micrometer;  // 35 microns for Copper

// Step 3: Define the radii for the circular windows
// The windows should fit inside the most negative ring (use the same inner radius as the ring)
G4double windowRadius = GasRadius;  // Use GasRadius for the window radii to fit inside the ring

// Step 4: Create the solid geometries for the Kapton and Copper windows
G4Tubs* solidKaptonWindow = new G4Tubs("solidKaptonWindow", 0, windowRadius, kaptonThickness / 2, 0, 360 * degree);
G4Tubs* solidCopperWindow = new G4Tubs("solidCopperWindow", 0, windowRadius, copperThickness / 2, 0, 360 * degree);

// Step 5: Create the logical volumes for the windows
G4LogicalVolume* logicKaptonWindow = new G4LogicalVolume(solidKaptonWindow, Kapton, "logicKaptonWindow");
G4LogicalVolume* logicCopperWindow = new G4LogicalVolume(solidCopperWindow, Copper, "logicCopperWindow");

// Step 6: Set visualization attributes (light orange for Kapton, orange for Copper)
G4VisAttributes* KaptonVisAttributes = new G4VisAttributes(G4Colour(1.0, 0.8, 0.0, 1.0));  // Light orange
KaptonVisAttributes->SetForceSolid(true);
logicKaptonWindow->SetVisAttributes(KaptonVisAttributes);

G4VisAttributes* CopperVisAttributes = new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 1.0));  // Orange
CopperVisAttributes->SetForceSolid(true);
logicCopperWindow->SetVisAttributes(CopperVisAttributes);

// Step 7: Position the windows at the same X position as the most negative ring (xPosNegativeRing)
// Place the Kapton window first, then the Copper window on top of it

// Kapton window placement
new G4PVPlacement(rotY,                                   // No rotation
                  G4ThreeVector(xPosNegativeRing, 0, 0),     // Positioned at xPosNegativeRing along X-axis
                  logicKaptonWindow,                         // Logical volume of the Kapton window
                  "KaptonWindow",                            // Name
                  logicWorld,                                // Mother volume (world)
                  false,                                     // No boolean operations
                  0);                                        // Copy number

// Copper window placement (shifted slightly along X-axis to avoid overlap)
new G4PVPlacement(rotY,                                   // No rotation
                  G4ThreeVector(xPosNegativeRing + kaptonThickness / 2 + copperThickness / 2, 0, 0),  // Positioned just after Kapton window
                  logicCopperWindow,                         // Logical volume of the Copper window
                  "CopperWindow",                            // Name
                  logicWorld,                                // Mother volume (world)
                  false,                                     // No boolean operations
                  0);                                        // Copy number

//! Source

//global translation for the source
G4double zTranslation = GasRadius + radialRingThickness + sourceDepth +2.*collimatorDepth + collimatorDistance;

  G4Material* Alluminium = nist->FindOrBuildMaterial("G4_Al");
  G4Material* Strontium = nist->FindOrBuildMaterial("G4_Sr");
  G4Material* Tungsten = nist->FindOrBuildMaterial("G4_W");

  // Outer cylindrical tube for the source container
  G4Tubs *OuterTube = new G4Tubs("outerSupport", 0, outerSourceRad, InnerSourceContThick / 2, 0, 360);

  // Inner cylindrical tube to subtract from the outer tube (creating the hollow part)
  G4Tubs *InnerTube = new G4Tubs("innerSupport", 0, innerSourceRad, sourceDepth / 2, 0, 360);

  // Subtracting the inner tube from the outer tube to create the hollow container
  G4VSolid* solidSourceContainer = new G4SubtractionSolid("SourceContainer", OuterTube, InnerTube, 0, G4ThreeVector(0., 0., InnerSourceContThick / 2));

  // Creating a logical volume for the source container (made of Aluminum)
  G4LogicalVolume* logicSourceContainer = new G4LogicalVolume(solidSourceContainer, Alluminium, "SourceContainer");

  // Setting visualization attributes for the source container
  G4Colour BaseColor(0.5, 0.5, 0.5,0.3);
  G4VisAttributes* baseVisAttributes = new G4VisAttributes(BaseColor);
  baseVisAttributes->SetForceSolid(true);
  logicSourceContainer->SetVisAttributes(baseVisAttributes);

  // Placing the source container into the world volume
  G4VPhysicalVolume* physSourceContainer = new G4PVPlacement(0,                      // No rotation
                                                            G4ThreeVector(0,0,-zTranslation),         // Placed at (0,0,0)
                                                            logicSourceContainer,    // Logical volume of the source container
                                                            "SourceContainer",       // Name
                                                            logicWorld,              // Mother volume (world)
                                                            false,                   // No boolean operations
                                                            0);                      // Copy number

  //
  // Defining the source region
  //

  // Setting the color and visualization attributes for the source (yellow)
  G4Colour SourceColor = G4Colour::Yellow();
  G4VisAttributes* SourceVisAttributes = new G4VisAttributes(SourceColor);
  SourceVisAttributes->SetForceSolid(true);

  // Define the depth of the source region
  SetSourceWidth(sourceDepth / 2);

  // Creating a solid cylindrical shape for the radioactive source (Strontium)
  G4Tubs *SolidSource = new G4Tubs("Source", 0, innerSourceRad, sourceDepth / 2, 0, 360);

  // Creating a logical volume for the radioactive source (made of Strontium)
  G4LogicalVolume* logicSource = new G4LogicalVolume(SolidSource, Strontium, "Source");

  // Applying the yellow visualization attributes to the source
  logicSource->SetVisAttributes(SourceVisAttributes);

  // Placing the radioactive source inside the hollow container, offset to fit within the container
  G4VPhysicalVolume* physiSource = new G4PVPlacement(0,                                    // No rotation
                                                    G4ThreeVector(0, 0, InnerSourceContThick / 2 - sourceDepth / 2 - zTranslation),  // Positioned inside the source container
                                                    logicSource,                             // Logical volume of the source
                                                    "Source",                                // Name
                                                    logicWorld,                              // Mother volume (world)
                                                    false,                                   // No boolean operations
                                                    0);    

  //
  // Adding the collimator in front of the source
  //

  // Defining the collimator visualization attributes (light brown color with 50% transparency)
  G4Colour collimatorColor(0.45, 0.25, 0.0, 0.5);
  G4VisAttributes* collimatorVisAttributes = new G4VisAttributes(collimatorColor);
  collimatorVisAttributes->SetForceSolid(true);

  // Creating the outer collimator cylinder
  G4Tubs *OuterCollimator = new G4Tubs("OutCollimator", 0, collimatorRadius, collimatorDepth / 2, 0, 360);

  // Creating the inner collimator hole cylinder
  G4Tubs *InnerCollimator = new G4Tubs("InnCollimator", 0, collimatorHoleRadius, collimatorDepth, 0, 360);

  // Subtracting the inner hole from the outer collimator to create the hollow collimator
  G4VSolid* SolidCollimator = new G4SubtractionSolid("Collimator", OuterCollimator, InnerCollimator, 0, G4ThreeVector(0., 0., 0.));

  // Creating a logical volume for the collimator (made of Tungsten)
  G4LogicalVolume* logicCollimator = new G4LogicalVolume(SolidCollimator, Tungsten, "Collimator");

  // Setting the visualization attributes for the collimator
  logicCollimator->SetVisAttributes(collimatorVisAttributes);

  // Placing the collimator in front of the source
  G4VPhysicalVolume* physCollimator = new G4PVPlacement(0,                      // No rotation
                                                        G4ThreeVector(0, 0, InnerSourceContThick / 2 + collimatorDepth / 2 + collimatorDistance - zTranslation),  // Positioned in front of the source
                                                        logicCollimator,         // Logical volume of the collimator
                                                        "Collimator",            // Name
                                                        logicWorld,              // Mother volume (world)
                                                        false,                   // No boolean operations
                                                        0);                      // Copy number

//! Sensitive Detector

  G4Colour GasColor(0.0,0.0,1.0,0.5);
  G4VisAttributes* GasVisAttributes = new G4VisAttributes(GasColor);

  G4Tubs* solidGasVolume = new G4Tubs("GasVolume",0,GasRadius,GasThickness/2,0,360);

  fLogicalGasVolume = new G4LogicalVolume(solidGasVolume,
            CYGNO_gas,
            "GasVolume"
            );
  
  fLogicalGasVolume->SetVisAttributes(GasVisAttributes);
  GasVisAttributes->SetForceSolid(true);

  G4VPhysicalVolume* GasVolume = new G4PVPlacement(rotY,
                  G4ThreeVector(0,0,0),
                  fLogicalGasVolume,
                  "GasVolume",
                  logicWorld,
                  false,
                  0);
/*
  // Create an instance of the sensitive detector
  GasBoxSensitiveDetector* GasBoxSD = new GasBoxSensitiveDetector("GasBoxSD");
  // Register the sensitive detector with the SD manager
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  SDManager->AddNewDetector(GasBoxSD);
  // Assign the sensitive detector to logicGasBox
  fLogicalGasVolume->SetSensitiveDetector(GasBoxSD);
*/
//! detector Alu outer box
  /*
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
*/

//! entering side
  /*
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
*/

//!Collimator
/*
  //
  
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

//! detector gas box
  /*
  

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
    logicGasBoxVis->SetForceSolid(true);
    logicGasBox->SetVisAttributes(logicGasBoxVis);

  new G4PVPlacement(nullptr,  // no rotation
    gasPos,                     // at position
    logicGasBox,              // its logical volume
    "gasBox",                 // its name
    logicWorld,                 // its mother  volume
    false,                    // no boolean operation
    0,                        // copy number
    checkOverlaps);           // overlaps checking

*/

  //
  //Creating the physicalvolumestore
  //
  
  fPhysVolStore = G4PhysicalVolumeStore::GetInstance();

  //
  //always return the physical World
  //
  return physWorld;
}


void DetectorConstruction::ConstructSDandField()
{
    // Create the sensitive detector
    if (!fSensitiveDetector) {
        fSensitiveDetector = new GasBoxSensitiveDetector("GasBoxSD");
    }

    // Ensure the logical gas volume exists before assigning the sensitive detector
    if (fLogicalGasVolume) {
        fLogicalGasVolume->SetSensitiveDetector(fSensitiveDetector);

        // Register the sensitive detector with the SD manager
        G4SDManager* SDManager = G4SDManager::GetSDMpointer();
        SDManager->AddNewDetector(fSensitiveDetector);
    } else {
        G4cerr << "Error: fLogicalGasVolume is not defined!" << G4endl;
    }
}