#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4VUserDetectorConstruction.hh>
#include "GasBoxSensitiveDetector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"

class G4LogicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  // Constructor
  DetectorConstruction();
  ~DetectorConstruction();
  // Main method that has to be overridden in all detectors
  // You will edit this method in Task 1a & Task 1b
  G4VPhysicalVolume* Construct() override;

  //variables referring to the detectorBox
  G4double GetDetHalfWidth() const { return 0.5 * detWidth; }
  G4double GetDetHalfHeight() const { return 0.5 * detHeight; }
  G4double GetDetHalfThickness() const { return 0.5 * detThickness; }

private:
  virtual void ConstructSDandField();
  G4PhysicalVolumeStore* fPhysVolStore;
  G4LogicalVolume* fLogicalGasVolume;
  GasBoxSensitiveDetector* fSensitiveDetector;

  void SetSourceWidth(G4double a) {fSourceWidth=a;}
  G4double fSourceWidth;
  // An example geometry created for you to finish task 0
  void ConstructDemo(G4LogicalVolume* worldLog);
  // Member variables for dimensions
  //!GAS CREATION
  G4double gasPressure;
  G4double He_frac;
  G4double CF4_frac;
  G4double Ar_frac;
  G4double gasThickness;
  G4double gasWidth;
  G4double gasHeight;
  //! plexiglass box
  G4double plexiSideThickness;
  G4double plexiWidth;
  G4double plexiHeight;
  G4double plexiThickness;
  G4double windowThickness;
  //! Field Cage
  G4double radialRingThickness;
  G4double RingThicknessAlongDrift;  
  G4double GasRadius;
  G4double GasThickness;
  G4double trenchHeight;
  G4double trenchMinRadius;    
  G4int Nrings;    
  G4double ringSpacing;    
  //!90Sr source
  G4double outerSourceRad;
  G4double innerSourceRad;
  G4double InnerSourceContThick;
  G4double sourceDepth;
  G4double collimatorRadius;
  G4double collimatorDepth;
  G4double collimatorHoleRadius;
  G4double collimatorDistance;
  G4double additionalDistance;
  //OLD  
  //! detector Alu outer box 
  G4double detWidth;
  G4double detHeight;
  G4double detThickness;
  G4double world_radius;
  G4double openSideThickness;

};

#endif