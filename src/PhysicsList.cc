#include "PhysicsList.hh"
#include <G4EmStandardPhysics.hh>
#include <G4EmExtraPhysics.hh>
#include <G4HadronElasticPhysics.hh>
#include <G4HadronPhysicsFTFP_BERT.hh>
#include <G4EmStandardPhysics_option3.hh>
#include <G4EmStandardPhysics_option1.hh>
#include <G4EmStandardPhysics_option2.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4DecayPhysics.hh>
#include <G4ProductionCutsTable.hh>
#include <G4SystemOfUnits.hh>
#include <G4EmLivermorePolarizedPhysics.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <G4EmPenelopePhysics.hh>
#include <G4EmParameters.hh>
#include <G4HadronElasticPhysicsHP.hh>

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  // Default Decay Physics
  RegisterPhysics(new G4DecayPhysics());
  // Default Radioactive Decay Physics
  RegisterPhysics(new G4RadioactiveDecayPhysics());
  //EM Physics
  RegisterPhysics(new G4EmStandardPhysics_option4(1));//recommended option
  RegisterPhysics(new G4EmExtraPhysics());
  //hadronic
  RegisterPhysics( new G4HadronElasticPhysicsHP(1) );
  //better low energy EM and polarization
  RegisterPhysics(new G4EmLivermorePolarizedPhysics());

  // Em options
  G4EmParameters* emPar = G4EmParameters::Instance();
  emPar->SetFluo(true);
  emPar->SetAuger(true);
  emPar->SetPixe(true);

  defaultCutValue = 0.001*mm;
  cutForGamma     = defaultCutValue;
  cutForElectron  = defaultCutValue;
  cutForPositron  = defaultCutValue;

}

void PhysicsList::SetCuts()
{
  // The method SetCuts() is mandatory in the interface. Here, one just use
  // the default SetCuts() provided by the base class.
  G4VUserPhysicsList::SetCuts();
  SetCutValue(cutForGamma, "gamma");
  SetCutValue(cutForElectron, "e-");
  SetCutValue(cutForPositron, "e+");

  DumpCutValuesTable();
}
