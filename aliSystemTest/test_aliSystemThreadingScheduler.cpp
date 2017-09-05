#include "gtest/gtest.h"
#include <aliSystem.hpp>
#include <vector>

namespace {

  const double ms     = 0.001; // 1 ms
  using Scheduler     = aliSystem::Threading::Scheduler;
  using Time          = aliSystem::Time;
  using TVec          = std::vector<Time::TP>;
  using Stats         = aliSystem::Stats;
  using Pool          = aliSystem::Threading::Pool;
  using Queue         = aliSystem::Threading::Queue;
  using Work          = aliSystem::Threading::Work;
    
}

TEST(aliSystemThreadingScheduler, general) {
  TVec       triggered;
  TVec       expected;
  bool       requeue = false;
  Pool::Ptr  pool    = Pool::Create("testSchedulingPool", 1);
  Queue::Ptr queue   = pool->AddQueue("testSchedulingQueue",
				      1,
				      Stats::Create("scheduling test stats"));
  Stats::Ptr workStats = Stats::Create("scheduler test stats");
  Work::Ptr  work      = Work::Create(workStats,
				      [&](bool &) {
					triggered.push_back(Time::Now());
				      });
  Time::TP t0 = Time::Now();
  Time::TP t1 = t0 + std::chrono::milliseconds(250);
  Time::TP t2 = t0 + std::chrono::milliseconds(500);
  work->Run(requeue);
  Scheduler::Schedule(queue, t1, work);
  Scheduler::Schedule(t2, work);
  expected.push_back(t0);
  expected.push_back(t1);
  expected.push_back(t2);
  sleep(1);
  ASSERT_EQ(triggered.size(), 3u);
  ASSERT_EQ(expected .size(), 3u);
  sleep(5);
  for (size_t i=0;i<3;++i) {
    Time::TP tp0 = t0;
    Time::TP tp1 = expected[i];
    Time::TP tp2 = triggered[i];
    ASSERT_NEAR(Time::ToSeconds(tp1-tp0),
		Time::ToSeconds(tp2-tp0),
		ms) << " expected-d0 vs triggered-d0 i=" << i;
  }
}
