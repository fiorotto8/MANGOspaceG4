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
    G4double holeDiameter = 70 * mm;  // Circular hole diameter

    // Set the basic properties for the particles to be produced
    G4ParticleDefinition* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fGPS->SetParticleDefinition(gamma);

    /*
    //! eneregy and spatial distribution
    // Configure the position distribution
    G4SPSPosDistribution* positionDist = fGPS->GetCurrentSource()->GetPosDist();
    positionDist->SetPosDisType("Plane");  // Set to emit from a plane
    positionDist->SetPosDisShape("Circle");  // The shape of the plane is circular
    positionDist->SetRadius(holeDiameter / 2);  // Half the diameter for the radius
    positionDist->SetCentreCoords(G4ThreeVector(-20 * cm, 0, 0));  // Center of the disk at (20, 0, 0) cm
    positionDist->SetPosRot1(G4ThreeVector(0, 1, 0));  // Alignment vector along y-axis
    positionDist->SetPosRot2(G4ThreeVector(0, 0, 1));  // Alignment vector along z-axis

    // Configure the angular distribution to emit directly along the +x axis
    G4SPSAngDistribution* angularDist = fGPS->GetCurrentSource()->GetAngDist();
    angularDist->SetAngDistType("iso");
    angularDist->DefineAngRefAxes("angref1", G4ThreeVector(0, 0, 1));  // Reference axis along +z
    // Set theta at 90 degrees to align with +x, and phi at 0 degrees
    angularDist->SetMinTheta(0 * deg);  // Theta at 90 degrees
    angularDist->SetMaxTheta(0 * deg);  // No spread in Theta
    angularDist->SetMinPhi(90 * deg);     // Phi at 0 degrees
    angularDist->SetMaxPhi(90 * deg);


    G4SPSEneDistribution* energyDist = fGPS->GetCurrentSource()->GetEneDist();
    // Set the energy distribution type to linear
    energyDist->SetEnergyDisType("Lin");
    // Set the gradient and intercept for the linear energy distribution
    energyDist->SetGradient(.0);  // Corresponds to /gps/ene/gradient 1
    energyDist->SetInterCept(1.0 );  // Corresponds to /gps/ene/intercept 1 keV
    // Set the minimum and maximum energy
    energyDist->SetEmin(10.0 * keV);  // Minimum energy
    energyDist->SetEmax(100.0 * keV);  // Maximum energy
    */

    //! single energy radom spatial distribution
    // Set the energy to 17.45 keV
    fGPS->GetCurrentSource()->GetEneDist()->SetMonoEnergy(17.45 * keV);
    
    // Set the direction to (0, 1, 0)
    fGPS->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(G4ThreeVector(1., 0., 0.));
    
    // Set the spatial distribution on a circle of diameter 70 cm
    G4double radius = 35 * mm;  // Radius of the circle
    fGPS->GetCurrentSource()->GetPosDist()->SetPosDisType("Plane");
    fGPS->GetCurrentSource()->GetPosDist()->SetPosDisShape("Circle");
    fGPS->GetCurrentSource()->GetPosDist()->SetRadius(radius);
    fGPS->GetCurrentSource()->GetPosDist()->SetCentreCoords(G4ThreeVector(-100., 0., 0.));
    fGPS->GetCurrentSource()->GetPosDist()->SetPosRot1(G4ThreeVector(0., 1., 0.)); // Circle lies in the X-Z plane
    fGPS->GetCurrentSource()->GetPosDist()->SetPosRot2(G4ThreeVector(0., 0., 1.)); // Circle lies in the X-Z plane

    // Set linear polarization along the Y-axis (1, 0, 0)
    fGPS->GetCurrentSource()->SetParticlePolarization(G4ThreeVector(0., 1., 0.));

    //! Custom energy distribution from Nustar
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
    fGPS->GeneratePrimaryVertex(anEvent);
}