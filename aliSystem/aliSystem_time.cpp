#include <aliSystem_time.hpp>
#include <cmath>
#include <cstdio>


namespace {
  const long long nano = 1000*1000*1000;
}

namespace aliSystem {

  Time::TP Time::Now() {
    TP now = std::chrono::high_resolution_clock::now();
    return now;
  }
  double Time::ToSeconds(const Dur &dur) {
    std::chrono::nanoseconds ns = dur;
    return double(ns.count()/nano) + double(ns.count()%nano)/nano;
  }
  Time::Dur Time::FromSeconds(double delay) {
    Dur    rtn = Dur::zero();
    double i;
    double f = modf(delay, &i);
    rtn += std::chrono::seconds((long long)i);
    rtn += std::chrono::nanoseconds((long long)(f*nano));
    return rtn;
  }

  time_t Time::ToTime(const TP &tp) {
    Dur dur = tp.time_since_epoch();
    std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
    return (time_t)sec.count();
  }

  Time::TP Time::FromTime(const time_t &t) {
    std::chrono::seconds sec = std::chrono::seconds((long long)t);
    Dur dur = std::chrono::duration_cast<Dur>(sec);
    return TP(dur);
  }

  struct timespec Time::ToTimeSpec(const TP &tp) {
    struct timespec rtn;
    Dur dur = tp.time_since_epoch();
    std::chrono::seconds     sec  = std::chrono::duration_cast<std::chrono::seconds>(dur);
    std::chrono::nanoseconds nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur)-sec;
    rtn.tv_sec  = sec.count();;
    rtn.tv_nsec = nsec.count();
    return rtn;
  }
    
  Time::TP Time::FromTimeSpec(const struct timespec &ts) {
    std::chrono::seconds     sec(ts.tv_sec);
    std::chrono::nanoseconds ns(ts.tv_nsec);
    return TP(sec + ns);
  }
    
}

std::ostream &operator<<(std::ostream &out, const aliSystem::Time::TP &tp) {
  return out << tp.time_since_epoch();
}

std::ostream &operator<<(std::ostream &out, const aliSystem::Time::Dur &dur) {
  std::chrono::nanoseconds ns   = dur;
  long long                tm   = ns.count();
  char                     buf[25];
  snprintf(buf, sizeof(buf), "%lli.%06lli", tm/nano, (tm%nano)/1000);
  return out << buf;
  
}
    
