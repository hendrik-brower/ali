#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Hold          = aliSystem::Hold;
  using Stats         = aliSystem::Stats;
  using Scheduler     = aliSystem::Threading::Scheduler;
  using Work          = aliSystem::Threading::Work;
  using Time          = aliSystem::Time;
  namespace Threading = aliSystem::Threading;
}

TEST(aliSystemHold, order) {
  std::string name = "holdTest";
  Hold::Ptr h1 = Hold::Create(__FILE__,    __LINE__, name);
  Hold::Ptr h2 = Hold::Create(__FILE__,    __LINE__, name);
  Hold::Ptr h3 = Hold::Create("Somethign", __LINE__, name);
  ASSERT_EQ(3+h1->GetLine(), (size_t)__LINE__);
  ASSERT_EQ(3+h2->GetLine(), (size_t)__LINE__);
  ASSERT_EQ(3+h3->GetLine(), (size_t)__LINE__);
  ASSERT_STREQ(h1->GetFile().c_str(), h2->GetFile().c_str());
  ASSERT_STRNE(h1->GetFile().c_str(), h3->GetFile().c_str());
  ASSERT_STREQ(name.c_str(), h1->GetReason().c_str());
  ASSERT_STREQ(name.c_str(), h2->GetReason().c_str());
  ASSERT_STREQ(name.c_str(), h3->GetReason().c_str());
  Time::TP     start = Time::Now();
  Stats::Ptr   stats = Stats::Create("hold testing stats");
  Scheduler::Schedule(start + std::chrono::seconds(1),
		      Work::Create(stats, [&](bool &) {
			  h1.reset();
			  h2.reset();
			  h3.reset();
			}));
  Hold::WaitForHolds();
  Time::TP end = Time::Now();
  ASSERT_TRUE(end-start >std::chrono::seconds(1));
}
