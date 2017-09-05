#include <aliSystem_stats.hpp>
#include <aliSystem_logging.hpp>
#include <cstring>

namespace aliSystem {

  Stats::Ptr Stats::Create(const std::string &name) {
    Ptr rtn(new Stats(name));
    return rtn;
  }
  Stats::Stats(const std::string &name_)
    : name(name_),
      count(0),
      runTime(Time::Dur::zero()) {
  }
  Stats::~Stats() {}
  const std::string &Stats::Name            () const { return name; }
  size_t             Stats::Count           () const { return count;   }
  Time::Dur          Stats::RunTime         () const { return runTime; }
  void Stats::Inc(const Time::Dur &timeInc) {
    std::lock_guard<std::mutex> g(lock);
    ++count;
    runTime += timeInc;
  }
  std::ostream &operator<<(std::ostream &out, const Stats &o) {
    out << "Stats(name=" << o.name
	<< ", count="    << o.count
	<< ", runTime="  << Time::ToSeconds(o.runTime)
	<< ")";
    return out;
  }
  
}


