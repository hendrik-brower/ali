#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  
  using Exec           = aliLuaCore::Exec;
  using ExecEngine     = aliLuaExt::ExecEngine;
  using ExecEngineWork = aliLuaExt::ExecEngineWork;
  using Future         = aliLuaCore::Future;
  using LuaFn          = aliLuaCore::LuaFn;
  using BPtr           = aliLuaTest::Util::BPtr;
  using Pool           = aliSystem::Threading::Pool;
  using Stats          = aliSystem::Stats;
  using Work           = aliSystem::Threading::Work;
  
}

TEST(aliLuaExtExecEngineWork, general) {
  Stats::Ptr      stats = Stats::Create("myStats");
  Pool::Ptr       pool = Pool::Create("test engine pool", 1);
  ExecEngine::Ptr exec = ExecEngine::Create("test engine work", pool);
  Future::Ptr     fPtr = Future::Create();
  BPtr            execMatch(new bool(false));
  BPtr            futureMatch(new bool(false));
  BPtr            luaMatch(new bool(false));
  bool            requeue = true;
  LuaFn           luaFn = [=](lua_State*) {
    *luaMatch = true;
    return 0;
  };
  ExecEngineWork::RunFn runFn = [=](const Exec::Ptr   &ePtr,
				    const Future::Ptr &future,
				    const LuaFn       &fn) {
    *execMatch = exec.get()==ePtr.get();
    *futureMatch = fPtr.get()==future.get();
    fn(nullptr);
  };

  Work::Ptr work = ExecEngineWork::Create(stats, exec, luaFn, fPtr, runFn);
  work->Run(requeue);
  ASSERT_FALSE(requeue);
  ASSERT_EQ(stats->Count(), 1u);
  ASSERT_TRUE(*execMatch);
  ASSERT_TRUE(*futureMatch);
  ASSERT_TRUE(*luaMatch);
}
