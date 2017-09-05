#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using InfoMap    = aliLuaCore::InfoMap;
  using LuaFn      = aliLuaCore::LuaFn;
  using MakeFn     = aliLuaCore::MakeFn;
  using MTU        = aliLuaCore::MakeTableUtil;
  using StatsOBJ   = aliLuaCore::Stats::OBJ;
  using Util       = aliLuaCore::Util;
  using Values     = aliLuaCore::Values;
  using BPtr       = aliLuaTest::Util::BPtr;
  using LPtr       = aliLuaTest::Util::LPtr;
  using TestUtil   = aliLuaTest::Util;
  using Stats      = aliSystem::Stats;
  using Pool       = aliSystem::Threading::Pool;

  const std::string dummyExecType = "execDummy";
  struct DummyExec : public Exec {
    using Ptr = std::shared_ptr<DummyExec>;
    using WPtr = std::weak_ptr<DummyExec>;
    static Ptr Create(const std::string &name) {
      Ptr  ptr(new DummyExec(name));
      WPtr wPtr = ptr;
      ptr->THIS = ptr;
      ptr->SetInfo("dummyInfo", [=](lua_State *L) {
	  MakeFn fn = GetInfo(wPtr.lock());
	  return fn(L);
	});
      return ptr;
    }
    static MakeFn GetInfo(const Ptr &) {
      MTU mtu;
      mtu.SetBoolean("isDummy", true);
      return mtu.GetMakeFn();
    }
    Exec::Ptr GetExec() const override {
      return THIS.lock();
    }
    bool IsBusy() const override {
      return isBusy;
    }
    size_t RunCount() const {
      return runCount;
    }
    const Listeners::Ptr &OnIdle() const override {
      return lPtr;
    }
  private:
    DummyExec(const std::string &name_)
      : Exec(dummyExecType, name_),
	isBusy(false),
	runCount(0) {
    }
    void InternalRun(const FuturePtr   &future,
		     const LuaFn       &luaFn) override {
      std::lock_guard<std::mutex> g(lock);
      ++runCount;
      isBusy = true;
      // The use of nullptr in the next call is simply
      // begging for a segmentation fault; however, this
      // is only for this extremely controlled test.
      luaFn(nullptr);
      if (future) {
	future->SetValue(Values::MakeNothing);
      }
      isBusy = false;
    }
    std::mutex     lock;
    WPtr           THIS;
    bool           isBusy;
    size_t         runCount;
    Listeners::Ptr lPtr;
  };
  
}

TEST(aliLuaCoreExec, general) {
  std::stringstream ss;
  const std::string name   = "dummy exec";
  DummyExec::Ptr    exec   = DummyExec::Create(name);
  Pool::Ptr         pool   = Pool::Create("pool", 1);
  ExecEngine::Ptr   engine = ExecEngine::Create("execEngine", pool);
  Stats::Ptr        stats  = Stats::Create("custom stats");
  Future::Ptr       fPtr   = Future::Create();
  LPtr              lPtr   = TestUtil::GetL();
  ASSERT_STREQ(dummyExecType.c_str(), exec->GetExecType().c_str());
  ASSERT_STREQ(name.c_str(), exec->Name().c_str());
  ASSERT_TRUE(exec->Stats());
  ASSERT_TRUE(exec->Registry());
  exec->SetStats(stats);
  ASSERT_EQ(exec->Stats().get(), stats.get());
  exec->Run(Values::MakeNothing);
  ASSERT_EQ(exec->RunCount(), 1u);
  exec->Run(fPtr, Values::MakeNothing);
  ASSERT_EQ(exec->RunCount(), 2u);
  ASSERT_TRUE(fPtr->IsSet());
  ss << *exec;
  ASSERT_STREQ(ss.str().c_str(), "Exec(dummy exec)");
  fPtr = Future::Create();
  Util::Run(engine, fPtr, [=](lua_State *L) {
      if (exec) {
	lua_checkstack(L,6);
	MakeFn info = exec->Exec::GetInfo();
	info(L);
	THROW_IF(lua_gettop(L)!=1, "InfoMap did not push 1 value");
	THROW_IF(!lua_istable(L,1), "InfoMap did not push a table");
	THROW_IF(LUA_TTABLE!=lua_getfield(L, 1, "execInfo"),
		 "execInfo field is not a table");
	THROW_IF(LUA_TTABLE!=lua_getfield(L, 1, "dummyInfo"),
		 "dummyInfo field is not a table");
	THROW_IF(LUA_TSTRING!=lua_getfield(L, 2, "execType"),
		 "execInfo does not include the string field 'execType'");
	THROW_IF(LUA_TSTRING!=lua_getfield(L, 2, "name"),
		 "execInfo does not include the string field 'name'");
	lua_getfield(L,2,"stats");
	THROW_IF(dummyExecType!=lua_tostring(L,-3),
		 "returned execType is not the expected value");
	THROW_IF(name!=lua_tostring(L,-2),
		 "returned name is not the expected value");
	Stats::Ptr sPtr = StatsOBJ::Get(L,-1,false);
	THROW_IF(LUA_TBOOLEAN!=lua_getfield(L,3,"isDummy"),
		 "dummyInfo does not include 'isDummy' field");
	THROW_IF(lua_toboolean(L,-1)!=true,
		 "failed to get the right value for dummyInfo.isDummy");
      }
      return 0;
    });
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

TEST(aliLuaCoreExec, scriptInterface) {
  Pool::Ptr         pool   = Pool::Create("pool", 1);
  ExecEngine::Ptr   engine = ExecEngine::Create("execEngine", pool);
  Stats::Ptr        stats  = Stats::Create("myStats");
  Future::Ptr       fPtr   = Future::Create();
  engine->SetStats(stats);
  Util::LoadString(engine, fPtr,
		   "-- test exec script interface"
		   "\n local exec = lib.aliLua.exec.GetEngine()"
		   "\n assert(exec, 'unable to retrieve the exec')"
		   "\n local info = exec:GetInfo()"
		   "\n assert(info, 'exec:GetInfo() call failed')"
		   "\n assert(type(info.execInfo)=='table', 'execInfo is not a table')"
		   "\n local stats = info.execInfo.stats"
		   "\n assert(stats, 'failed to retrieve the stats')"
		   "\n local sInfo = stats:GetInfo()"
		   "\n assert(sInfo.name=='myStats', 'unexpected stats object')"
		   "\n exec:SetStats(lib.aliLua.stats.Create('myNewStats'))"
		   "\n info = exec:GetInfo()"
		   "\n stats = info.execInfo.stats"
		   "\n sInfo = stats:GetInfo()"
		   "\n assert(sInfo.name=='myNewStats', 'reassignment of stats failed')"
		   "\n");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}
