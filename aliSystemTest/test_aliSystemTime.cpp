#include "gtest/gtest.h"
#include <aliSystem.hpp>
#include <time.h>

namespace {
  using Time = aliSystem::Time;
}

TEST(aliSystemTime, Now) {
  time_t               a;
  time_t               b;
  std::chrono::seconds sec;
  Time::TP             now = Time::Now();
  time(&a);
  b = aliSystem::Time::ToTime(now);
  ASSERT_NEAR((long long)a, (long long)b, 1);
}

TEST(aliSystemTime, ToFromSeconds) {
  double    us              = 0.000001;
  Time::TP  now             = Time::Now();
  double    nowInSeconds    = Time::ToSeconds(now.time_since_epoch());
  Time::Dur offset          = Time::FromSeconds(nowInSeconds);
  double    offsetInSeconds = Time::ToSeconds(offset);
  ASSERT_NEAR(nowInSeconds, offsetInSeconds, us);
}

TEST(aliSystemTime, ToFromTime) {
  Time::TP  now             = Time::Now();
  time_t    nowInTimeT      = Time::ToTime(now);
  Time::TP  nowFromTimeT    = Time::FromTime(nowInTimeT);
  double    a               = Time::ToSeconds(now.time_since_epoch());
  double    b               = Time::ToSeconds(nowFromTimeT.time_since_epoch());
  ASSERT_NEAR(a,b,1);
}

TEST(aliSystemTime, ToFromTimeSpec) {
  Time::TP        now             = Time::Now();
  struct timespec nowInTimeSpec   = Time::ToTimeSpec(now);
  Time::TP        nowFromTimeSpec = Time::FromTimeSpec(nowInTimeSpec);
  double          a               = Time::ToSeconds(now.time_since_epoch());
  double          b               = Time::ToSeconds(nowFromTimeSpec.time_since_epoch());
  ASSERT_NEAR(a,b,1);
}
