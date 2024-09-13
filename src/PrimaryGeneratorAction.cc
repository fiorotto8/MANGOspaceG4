#include "PrimaryGeneratorAction.hh"

#include <G4ParticleTable.hh>
#include <G4Event.hh>
#include <G4SystemOfUnits.hh>
#include <G4GeneralParticleSource.hh>
#include <G4ParticleGun.hh>
#include <G4SPSAngDistribution.hh>
#include <Randomize.hh>
#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4VSolid.hh>
#include <G4Box.hh>
#include <G4SPSPosDistribution.hh>
#include <G4SPSAngDistribution.hh>
#include <DetectorConstruction.hh>
#include <G4RunManager.hh>

// Task 2b.1 Include the proper header file for GPS

using namespace std;

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    // Instantiate a GPS
    fGPS = new G4GeneralParticleSource();
    G4double holeDiameter = 10.5 * mm;  // Circular hole diameter

    // Set the basic properties for the particles to be produced
    G4ParticleDefinition* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fGPS->SetParticleDefinition(gamma);

    // Configure the position distribution
    G4SPSPosDistribution* positionDist = fGPS->GetCurrentSource()->GetPosDist();
    positionDist->SetPosDisType("Plane");  // Set to emit from a plane
    positionDist->SetPosDisShape("Circle");  // The shape of the plane is circular
    positionDist->SetRadius(holeDiameter / 2);  // Half the diameter for the radius
    positionDist->SetCentreCoords(G4ThreeVector(20 * cm, 0, 0));  // Center of the disk at (20, 0, 0) cm
    positionDist->SetPosRot1(G4ThreeVector(0, 1, 0));  // Alignment vector along y-axis
    positionDist->SetPosRot2(G4ThreeVector(0, 0, 1));  // Alignment vector along z-axis

    // Configure the angular distribution to emit directly along the +x axis
    G4SPSAngDistribution* angularDist = fGPS->GetCurrentSource()->GetAngDist();
    angularDist->SetAngDistType("iso");
    angularDist->DefineAngRefAxes("angref1", G4ThreeVector(0, 0, 1));  // Reference axis along +z
    // Set theta at 90 degrees to align with +x, and phi at 0 degrees
    angularDist->SetMinTheta(180 * deg);  // Theta at 90 degrees
    angularDist->SetMaxTheta(180 * deg);  // No spread in Theta
    angularDist->SetMinPhi(90 * deg);     // Phi at 0 degrees
    angularDist->SetMaxPhi(90 * deg);


    G4SPSEneDistribution* energyDist = fGPS->GetCurrentSource()->GetEneDist();
    // Set the energy distribution type to linear
    energyDist->SetEnergyDisType("Lin");
    // Set the gradient and intercept for the linear energy distribution
    energyDist->SetGradient(.0);  // Corresponds to /gps/ene/gradient 1
    energyDist->SetInterCept(1.0 );  // Corresponds to /gps/ene/intercept 1 keV
    // Set the minimum and maximum energy
    energyDist->SetEmin(1.0 * keV);  // Minimum energy
    energyDist->SetEmax(10.0 * keV);  // Maximum energy

    // Custom energy distribution from Nustar
    /*
    fGPS->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Arb");
    fGPS->GetCurrentSource()->GetEneDist()->ArbEnergyHistoFile("CRAB_NuStar.dat");
    fGPS->GetCurrentSource()->GetEneDist()->ArbInterpolate("Spline");
    */
    // Optional: Set monoenergetic
    //fGPS->GetCurrentSource()->GetEneDist()->SetMonoEnergy(2*keV);


    /* Code to launch particles from a disk that emits isotopically inside a certain angle range
    // Set emission from a circle
    G4double emissionRadius = 0.875*mm, emissionPos = (1.*m+halfThickness);
    G4SPSPosDistribution* posDist = fGPS->GetCurrentSource()->GetPosDist();
    posDist->SetPosDisType("Plane"); // Set distribution type to Plane
    posDist->SetPosDisShape("Circle"); // Set shape to Circle (disk)
    posDist->SetRadius(emissionRadius); // Set the radius of the disk
    posDist->SetCentreCoords(G4ThreeVector(1.*m, 0.*m, 0.*m)); // Set center of the disk

    //fix the emission angles to shoot only isotopically in the gas volume
    //! The particles are shot in a circle Inscribed in the Square of the detector box volume!!!!!
    G4SPSAngDistribution* angDist = fGPS->GetCurrentSource()->GetAngDist();
    angDist->SetAngDistType("iso"); // Set isotropic distribution
    G4double sourceTh0=90*deg, sourcePhi0=0*deg;
    G4double sourceSpan=std::tan(halfHeight/emissionPos);
    angDist->SetMinTheta(sourceTh0-sourceSpan);
    angDist->SetMaxTheta(sourceTh0+sourceSpan);
    angDist->SetMinPhi(sourcePhi0-sourceSpan);
    angDist->SetMaxPhi(sourcePhi0+sourceSpan);
    */

    /*Code for a source that emits from a point in a certain angle range
    //Set source position
    fGPS->GetCurrentSource()->GetPosDist()->SetCentreCoords(G4ThreeVector(1.*m,0,0));
    // Set isotropic angular distribution with limits
    fGPS->GetCurrentSource()->GetAngDist()->SetAngDistType("iso");
    G4double sourceTh0=90*deg, sourcePhi0=0*deg;
    G4double sourceSpan=0.5*deg;
    fGPS->GetCurrentSource()->GetAngDist()->SetMinTheta(sourceTh0-sourceSpan);
    fGPS->GetCurrentSource()->GetAngDist()->SetMaxTheta(sourceTh0+sourceSpan);
    fGPS->GetCurrentSource()->GetAngDist()->SetMinPhi(sourcePhi0-sourceSpan);
    fGPS->GetCurrentSource()->GetAngDist()->SetMaxPhi(sourcePhi0+sourceSpan);
    */
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    //Delete the GPS
    delete fGPS;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // Task 2a.2: Include the position randomization
    // Task 2b.1: Comment out all previous commands in this method (there is no fGun!)
    // Task 2b.1: The method for vertex creation remains the same,.just change the object to your GPS
    fGPS->GeneratePrimaryVertex(anEvent);
}