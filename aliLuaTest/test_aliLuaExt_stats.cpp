#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using Util       = aliLuaCore::Util;
  using LPtr       = aliLuaTest::Util::LPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
  using Stats      = aliSystem::Stats;
  
}


TEST(aliLuaCoreStats, statsInfo) {
  Stats::Ptr sPtr = Stats::Create("infoUtilTest");
  LPtr lPtr = TestUtil::GetL();
  lua_State *L = lPtr.get();
  sPtr->Inc(std::chrono::seconds(5));
  MakeFn info = aliLuaCore::Stats::Info(sPtr);
  info(L);
  ASSERT_TRUE(lua_istable(L,1));
  lua_getfield(L,1,"count");
  lua_getfield(L,1,"runTime");
  ASSERT_EQ(1,lua_tointeger(L,2)) << "verify count";
  ASSERT_EQ(5,lua_tointeger(L,3)) << "verify runTime";
}

TEST(aliLuaCoreStats, scriptInterface) {
  const std::string name = "stats tests";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr = Future::Create();
  Util::LoadString(exec,fPtr,
		   "-- stats script interfaces"
		   "\n function Verify(stats, name, count, runTime)"
		   "\n   info = stats:GetInfo()"
		   "\n   assert(info.name    == name,         'bad name')"
		   "\n   assert(info.count   == count,        'bad count')"
		   "\n   assert(info.runTime <= runTime+0.01, 'bad runTime - too high')"
		   "\n   assert(info.runTime >= runTime-0.01, 'bad runTime - too low')"
		   "\n end"
		   "\n local name = 'my stats'"
		   "\n local stats = lib.aliLua.stats.Create(name)"
		   "\n Verify(stats, name, 0, 0)"
		   "\n stats:Inc(3.4)"
		   "\n Verify(stats, name, 1, 3.4)"
		   "\n stats:Inc(1.9)"
		   "\n Verify(stats, name, 2, 3.4+1.9)"
		   "\n");
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
