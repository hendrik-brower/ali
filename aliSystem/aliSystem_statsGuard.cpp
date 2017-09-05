#include <aliSystem_statsGuard.hpp>


namespace aliSystem {

  StatsGuard::StatsGuard(Stats::Ptr sPtr_)
    : sPtr(sPtr_) {
    if (sPtr) {
      startTime = Time::Now();
    }
  }
  StatsGuard::~StatsGuard() {
    if (sPtr) {
      sPtr->Inc(Time::Now()-startTime);
    }
  }
    
}
