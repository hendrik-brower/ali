#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Stats = aliSystem::Stats;
  using Work  = aliSystem::Threading::Work;
  using IPtr  = std::shared_ptr<size_t>;
}

TEST(aliSystemThreadingWork, general) {
  bool        requeue  = false;
  IPtr        runCount(new size_t(0));
  Stats::Ptr  stats    = Stats::Create("work testing");
  Work::Ptr   work     = Work::Create(stats,
				      [=](bool &requeue) {
					requeue = (*runCount)%2;
					++(*runCount);
				      });
  ASSERT_EQ(work->GetStats().get(),stats.get());
  ASSERT_EQ(*runCount,0u) << "Run function not yet run";
  ASSERT_FALSE(requeue) << "verify initial requeue value";

  work->Run(requeue);
  ASSERT_EQ(*runCount,1u) << "first run";
  ASSERT_FALSE(requeue);

  work->Run(requeue);
  ASSERT_EQ(*runCount,2u) << "first run";
  ASSERT_TRUE(requeue);
}

