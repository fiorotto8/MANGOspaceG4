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


private:


};
#endif