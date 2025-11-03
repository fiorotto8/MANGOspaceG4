/// \file B3/B3a/src/PhysicsList.cc
/// \brief Implementation of the B3::PhysicsList class

#include "PhysicsList.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4SystemOfUnits.hh"

namespace B3
{


PhysicsList::PhysicsList()
{
  SetVerboseLevel(1);

  // Default physics
  RegisterPhysics(new G4DecayPhysics());

  // EM physics
  //RegisterPhysics(new G4EmStandardPhysics());

  // Radioactive decay
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  //RegisterPhysics(new G4EmLivermorePhysics());
  RegisterPhysics(new G4EmLivermorePolarizedPhysics());

}


void PhysicsList::SetCuts()
{
  // Global production cuts (applies everywhere unless a Region overrides)
  SetCutValue(0.001*mm, "gamma");
  SetCutValue(0.001*mm, "e-");
  SetCutValue(0.001*mm, "e+");

  G4VUserPhysicsList::SetCuts();
}


}
