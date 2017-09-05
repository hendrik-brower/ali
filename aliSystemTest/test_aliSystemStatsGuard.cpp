#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Stats      = aliSystem::Stats;
  using StatsGuard = aliSystem::StatsGuard;
  std::chrono::milliseconds ms(1);
  std::chrono::seconds      sec(1);
}

TEST(aliSystemStatsGuard, general) {
  Stats::Ptr stats(new Stats("statsItem"));
  {
    StatsGuard g(stats);
    sleep(1);
  }
  EXPECT_TRUE(stats->RunTime()-sec <  ms);
  EXPECT_EQ(stats->Count(), (size_t)1);
  {
    StatsGuard g(stats);
    sleep(1);
  }
  EXPECT_TRUE(stats->RunTime() - 2*sec <  2*ms);
  EXPECT_EQ(stats->Count(), (size_t)2);
}
