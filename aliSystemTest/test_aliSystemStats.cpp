#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Stats = aliSystem::Stats;
  using Time  = aliSystem::Time;
}

TEST(aliSystemStats, Basic) {
  std::chrono::microseconds  us(1);
  std::chrono::seconds       sec(1);
  size_t                     numCycles = 10;
  Stats                     *null      = nullptr;
  const std::string          name      = "my stats";
  Stats::Ptr                 stats     = Stats::Create(name);
  Time::Dur                  inc       = sec+us;
  ASSERT_NE(stats.get(), null); // change to null_ptr
  ASSERT_STREQ(name.c_str(), stats->Name().c_str());
  ASSERT_EQ(stats->Count(),(size_t)0);
  for (size_t i=0;i<numCycles;++i) {
    stats->Inc(inc);
  }
  ASSERT_EQ(stats->Count(),(size_t)numCycles);
  ASSERT_NEAR(Time::ToSeconds(stats->RunTime()), numCycles*Time::ToSeconds(inc), Time::ToSeconds(us));
}
