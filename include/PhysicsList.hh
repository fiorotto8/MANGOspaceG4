/// \file B3/B3a/include/PhysicsList.hh
/// \brief Definition of the B3::PhysicsList class

#ifndef B3PhysicsList_h
#define B3PhysicsList_h 1

#include "G4VModularPhysicsList.hh"

namespace B3
{

/// Modular physics list
///
/// It includes the folowing physics builders
/// - G4DecayPhysics
/// - G4RadioactiveDecayPhysics
/// - G4EmStandardPhysics

class PhysicsList: public G4VModularPhysicsList
{
public:
  PhysicsList();
  ~PhysicsList() override = default;

  void SetCuts() override;
};

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

