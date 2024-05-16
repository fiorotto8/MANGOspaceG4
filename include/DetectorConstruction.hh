#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4VUserDetectorConstruction.hh>

class G4LogicalVolume;

/**
  * Obligatory class responsible for geometry - volumes, materials, fields, etc.
  *
  * You will work mainly with this header file (.hh) and its associated source file (.cc).
  */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  // Constructor
    DetectorConstruction();
    // Main method that has to be overridden in all detectors
    // You will edit this method in Task 1a & Task 1b
    G4VPhysicalVolume* Construct() override;

    //variables referring to the detectorBox
    G4double GetDetHalfWidth() const { return 0.5 * detWidth; }
    G4double GetDetHalfHeight() const { return 0.5 * detHeight; }
    G4double GetDetHalfThickness() const { return 0.5 * detThickness; }

private:
    // An example geometry created for you to finish task 0
    void ConstructDemo(G4LogicalVolume* worldLog);
    // Member variables for dimensions
    G4double detWidth;
    G4double detHeight;
    G4double detThickness;
    G4double world_radius;
    G4double openSideThickness;
    G4double He_frac;
    G4double CF4_frac;
    G4double Ar_frac;
    G4double gasThickness;
    G4double gasWidth;
    G4double gasHeight;
};

#endif