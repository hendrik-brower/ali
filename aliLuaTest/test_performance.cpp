#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using Util       = aliLuaCore::Util;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
  using Stats      = aliSystem::Stats;
  using StatsGuard = aliSystem::StatsGuard;
  using Time       = aliSystem::Time;

  const int kilo = 1000;
  const int meg  = kilo * kilo;
}

TEST(aliLua, highLevelPerformance) {
  //
  // begin testing
  int             numRuns = 100 * kilo;
  std::string     name    = "performance";
  Stats::Ptr      stats   = Stats::Create(name);
  Pool::Ptr       pool    = Pool::Create(name,1);
  ExecEngine::Ptr exec    = ExecEngine::Create(name,pool);
  {
    Future::Ptr fPtr;
    StatsGuard  g(stats);
    for (int i=0;i<numRuns;++i) {
      fPtr = Future::Create();
      Util::Run(exec, fPtr, [=](lua_State *) {
	  static int runCount = 0;
	  ++runCount;
	  return 0;
	});
    }
    TestUtil::Wait(exec, fPtr);
  }
  //
  // Just looking for something reasonable.  Generally calls to/from Lua
  // are around ~1us for light weight calls.  This is intended to catch
  // queuing inefficiencies, inefficiencies in how Util::Run works as
  // well as using the aliLuaCore::Future to capture results.  In an early 
  // test, this routine  showed ~150K calls/second. In general, on average,
  // an application should be performing substantially more work within a
  // queued call, so this overhead should not represent a high percentage
  // of overall execution time.
  //
  double runTime        = Time::ToSeconds(stats->RunTime());
  double callsPerSecond = numRuns/runTime;
  //INFO("calls/second " << callsPerSecond);
  ASSERT_GE(callsPerSecond, 50*kilo);
}

TEST(aliLua, lowLevelPerformance) {
  //
  // begin testing
  int             numInc  = 100 * kilo;
  std::string     name    = "performance";
  Stats::Ptr      stats   = Stats::Create(name);
  Pool::Ptr       pool    = Pool::Create(name,1);
  ExecEngine::Ptr exec    = ExecEngine::Create(name,pool);
  {
    StatsGuard  g(stats);
    Future::Ptr fPtr = Future::Create();
    Util::Run(exec, fPtr, [=](lua_State *) {
	int runCount = 0;
	while (true) {
	  ++runCount;
	  if (runCount>numInc) {
	    break;
	  }
	}
	return 0;
      });
    TestUtil::Wait(exec, fPtr);
  }
  //
  // In this case, we are merely capturing the overhead of 1 Lua call doing
  // the same work as the highLevelPerformance test.  This design should
  // provide substantially higher performance.
  // Since this test is going to be ~ 10us + numInc * c++ inc time, the
  // execution will generally measure the cpu performance.  In inital tests,
  // ~85M increments/second were observed.  The test just specifies 10M inc/sec
  // as different processors could cause substantial variation of this
  // value.
  //
  double runTime      = Time::ToSeconds(stats->RunTime());
  double incPerSecond = numInc/runTime;
  //INFO("increments/second " << incPerSecond);
  ASSERT_GE(incPerSecond, 10*meg);
}

TEST(aliLua, luaPerformance) {
  //
  // begin testing
  int               numInc  = 100 * kilo;
  std::string       name    = "performance";
  Stats::Ptr        stats   = Stats::Create(name);
  Pool::Ptr         pool    = Pool::Create(name,1);
  ExecEngine::Ptr   exec    = ExecEngine::Create(name,pool);
  std::stringstream script;
  script <<
    "-- Lua performance test"
    "\n j=0"
    "\n while true do"
    "\n    j = j + 1"
    "\n    if j>=" << numInc << " then"
    "\n       break"
    "\n    end"
    "\n end";
  {
    StatsGuard  g(stats);
    Future::Ptr fPtr = Future::Create();
    Util::LoadString(exec, fPtr, script.str());
    TestUtil::Wait(exec,fPtr);
  }
  //
  // In this case, we are merely capturing the overhead of 1 Lua call doing
  // the same work as the highLevelPerformance test, but the work is done in
  // Lua.  This design should provide substantially higher performance than
  // the high level test, but lower performance than the low level test where
  // the work is done in c++ rather than the Lua virutal machine.
  // This test like the low level test will generally measure cpu performance.
  // In inital tests, ~18M increments/second were observed.  The test just
  // specifies 2M inc/sec as different processors could cause substantial
  // variation of this value.
  //
  double runTime         = Time::ToSeconds(stats->RunTime());
  double luaIncPerSecond = numInc/runTime;
  //INFO("increments/second " << luaIncPerSecond);
  ASSERT_GE(luaIncPerSecond, 2*meg);
}
