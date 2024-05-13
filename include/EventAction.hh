#ifndef EVENTACTION_HH
#define EVENTACTION_HH
#include <G4UserEventAction.hh>
#include <vector>

class EventAction : public G4UserEventAction {
public:
    EventAction();
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event*) override;
    virtual void EndOfEventAction(const G4Event*) override;
//primaries
    const std::vector<bool>& GetArrivedBooleans() const { return BoolArrived; }
    const std::vector<bool>& GetInteractedBooleans() const { return BoolInteracted; }
    const std::vector<double>& GetTotEnergyDeposit() const { return totalPrimaryEnergyDeposit; }
    const std::vector<double>& GetPrimaryEnergy() const { return primaryEnergy; }
    const std::vector<int>& GetPDGPrimary() const { return primaryPDG; }
//secondaries
    const std::vector<int>& GetNumSecondaries() const { return numSecondaries; }
    const std::vector<std::vector<int>>& GetPDGSecondaries() const { return secondariesPDG; }


private:
//primaries
    std::vector<bool> BoolArrived;
    std::vector<bool> BoolInteracted;
    std::vector<double> totalPrimaryEnergyDeposit;
    std::vector<double> primaryEnergy;
    std::vector<int> primaryPDG;
//secondaries
    std::vector<int> numSecondaries;
    std::vector<std::vector<int>> secondariesPDG;

};
#endif