#pragma once
#include "G4VUserTrackInformation.hh"

namespace B3a {

class TrackInfo : public G4VUserTrackInformation {
public:
  explicit TrackInfo(int primaryID = -1) : fPrimaryID(primaryID) {}
  ~TrackInfo() override = default;

  inline int  GetPrimaryID() const { return fPrimaryID; }
  inline void SetPrimaryID(int id) { fPrimaryID = id;  }

private:
  int fPrimaryID; // trackID of the primary ancestor for this track
};

} // namespace B3a
