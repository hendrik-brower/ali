#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <unistd.h>

// This module runs basic checks for:
//     aliLuaExt::Threading::PoolOBJ
//     aliLuaExt::Threading::QueueOBJ
//     aliLuaExt::Threading::WorkOBJ
// It does not do a thorough evaluation of
// the functional logic as it assumes aliSystemTest
// is covering those details.

namespace {
  using Exec        = aliLuaCore::Exec;
  using ExecEngine  = aliLuaExt::ExecEngine;
  using FunctionMap = aliLuaCore::FunctionMap;
  using Future      = aliLuaCore::Future;
  using Module      = aliLuaCore::Module;
  using PoolOBJ     = aliLuaExt::Threading::PoolOBJ;
  using QueueOBJ    = aliLuaExt::Threading::QueueOBJ;
  using WorkOBJ     = aliLuaExt::Threading::WorkOBJ;
  using StatsOBJ    = aliLuaCore::Stats::OBJ;
  using Util        = aliLuaCore::Util;
  using TestUtil    = aliLuaTest::Util;
  using Pool        = aliSystem::Threading::Pool;
  using Queue       = aliSystem::Threading::Queue;
  using Work        = aliSystem::Threading::Work;
  using Stats       = aliSystem::Stats;
  const int delay = 10*1000; // 10ms

  int GetWork(lua_State *L) {
    Stats::Ptr stats = StatsOBJ::Get(L,1,false);
    Work::Ptr  work  = Work::Create(stats, [](bool &requeue) {
	requeue = false;
	usleep(delay);
      });
    return WorkOBJ::Make(L,work);
  }
  
  void Init() {
    FunctionMap::Ptr fnMap = FunctionMap::Create("threading test functions");
    fnMap->Add("GetWork", GetWork);
    Module::Register("load threading test functions",
		     [=](const Exec::Ptr &ePtr) {
		       Util::LoadFnMap(ePtr, "lib.aliLuaTest.threading",  fnMap);
		     });
  }
  void Fini() {
  }

  struct ThreadTests : testing::Test {
    void SetUp() {
      name = "Lua threading tests";
      init =
	"--threading tests"
	"\n function EQ(info, key, val, prefix)"
	"\n    if info[key]~=val then"
	"\n       prefix = prefix or ''"
	"\n       error('error ' .. prefix"
	"\n             .. tostring(key) .. ' is expected to be '"
	"\n             .. tostring(val) .. ' got '"
	"\n             .. tostring(info[key]))"
	"\n    end"
	"\n end"
	"\n function GE(info, key, val, prefix)"
	"\n    if info[key]<val then"
	"\n       prefix = prefix or ''"
	"\n       error('error ' .. prefix"
	"\n             .. tostring(key) .. ' is expected to be greater than '"
	"\n             .. tostring(val) .. ' got '"
	"\n             .. tostring(info[key]))"
	"\n    end"
	"\n end"
	"\n function LE(info, key, val, prefix)"
	"\n    if info[key]>val then"
	"\n       prefix = prefix or ''"
	"\n       error('error ' .. prefix"
	"\n             .. tostring(key) .. ' is expected to be greater than '"
	"\n             .. tostring(val) .. ' got '"
	"\n             .. tostring(info[key]))"
	"\n    end"
	"\n end"
	"\n function ST(info, key, name, cnt, runTime, runDiff)"
	"\n    stats = info[key]"
	"\n    sInfo = stats:GetInfo()"
	"\n    keyDot = tostring(key) .. '.'"
	"\n    EQ(sInfo, 'name',    name,            keyDot)"
	"\n    EQ(sInfo, 'count',   cnt,             keyDot)"
	"\n    GE(sInfo, 'runTime', runTime-runDiff, keyDot)"
	"\n    LE(sInfo, 'runTime', runTime+runDiff, keyDot)"
	"\n end"
      	"\n function QEQ(queue, criteria)"
	"\n    qInfo = queue:GetInfo()"
	"\n    for key, val in pairs(criteria) do"
	"\n       EQ(qInfo, key, val, 'queue.')"
	"\n    end"
	"\n end"
	"\n "
	"\n pName      = 'myPool'"
	"\n qName      = 'myQueue'"
	"\n pStatsName = 'pool stats'"
	"\n qStatsName = 'queue stats'"
	"\n wStatsName = 'work stats'"
	"\n pStats = lib.aliLua.stats.Create(pStatsName)"
	"\n qStats = lib.aliLua.stats.Create(qStatsName)"
	"\n wStats  = lib.aliLua.stats.Create(wStatsName)"
	"\n pool = lib.aliLua.threading.CreatePool {"
	"\n    name       = pName,"
	"\n    numThreads = 1,"
	"\n    stats      = pStats,"
	"\n }"
	"\n queue = pool:AddQueue {"
	"\n   queueName      = qName,"
	"\n   maxConcurrency = 1,"
	"\n   stats          = qStats,"
	"\n }"
	"\n work = lib.aliLuaTest.threading.GetWork(wStats)";
      pool = Pool::Create(name, 1);
      exec = ExecEngine::Create(name, pool);
      fPtr = Future::Create();
    }
    void TearDown() {
      pool.reset();
      exec.reset();
      fPtr.reset();
    }
    std::string     name;
    std::string     init;
    Pool::Ptr       pool;
    ExecEngine::Ptr exec;
    Future::Ptr     fPtr;
  };
  
}

void RegisterInitFini_ThreadingTest(aliSystem::ComponentRegistry &cr) {
  aliSystem::Component::Ptr ptr = cr.Register("test_aliLuaExtThreading", Init, Fini);
  ptr->AddDependency("aliLuaCore");
}

TEST(aliLuaExtThreading, objects) {
  ASSERT_NE( PoolOBJ::GetPtr().get(), nullptr);
  ASSERT_NE(QueueOBJ::GetPtr().get(), nullptr);
  ASSERT_NE( WorkOBJ::GetPtr().get(), nullptr);
}

TEST_F(ThreadTests, poolTests) {
  Util::LoadString(exec, init);
  Util::LoadString(exec, fPtr, 
		   "\n pInfo = pool:GetInfo()"
		   "\n sInfo = pStats:GetInfo()"
   		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)" // letting threads start up
		   "\n EQ(pInfo, 'name',       pName)"
		   "\n EQ(pInfo, 'numThreads', 1)"
		   "\n EQ(sInfo, 'name',       pStatsName)"
		   "\n EQ(sInfo, 'count',      0)"
		   "\n EQ(sInfo, 'runTime',    0)"
		   "\n "
		   "\n -- run a couple work units"
		   "\n "
		   "\n queue:AddWork(work)"
		   "\n queue:AddWork(work)"
   		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)" // letting work finish
		   "\n sInfo = pStats:GetInfo()"
		   "\n EQ(sInfo, 'count',   2)"
		   "\n GE(sInfo, 'runTime', 0.019)"
		   "\n LE(sInfo, 'runTime', 0.021)"
		   "\n "
		   "\n -- check info"
		   "\n "
		   "\n pool:SetNumThreads(2)"
		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)"
		   "\n pInfo = pool:GetInfo()"
		   "\n EQ(pInfo, 'numThreads', 2)"
		   "\n");
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

TEST_F(ThreadTests, queueTests) {
  Util::LoadString(exec, init);
  Util::LoadString(exec, fPtr, 
		   "\n qInfo = queue:GetInfo()"
		   "\n expected = {"
		   "\n    name = qName,"
		   "\n    currentConcurrency = 0,"
		   "\n    isStopped = false,"
		   "\n    isBusy = false,"
		   "\n    isFrozen = false,"
		   "\n    numberPending = 0,"
		   "\n    maxConcurrency = 1,"
		   "\n }"
		   "\n QEQ(queue, expected)"
		   "\n ST(qInfo, 'queueStats',         qStatsName, 0, 0, 0)"
		   "\n "
		   "\n -- manipulate & verify"
		   "\n "
		   "\n queue:Stop()"
		   "\n expected.isStopped = true"
		   "\n QEQ(queue, expected)"
		   "\n "
		   "\n -- add some work"
		   "\n for i=1,10 do"
		   "\n    queue:AddWork(work)"
		   "\n end"
		   "\n expected.numberPending = 10"
		   "\n expected.isBusy        = true"
		   "\n QEQ(queue, expected)"
		   "\n queue:SetMaxConcurrency(10)"
		   "\n expected.maxConcurrency = 10" // still limited by pool
		   "\n QEQ(queue, expected)"
		   "\n queue:Freeze()"
		   "\n expected.isFrozen = true"
		   "\n QEQ(queue, expected)"
		   "\n queue:Start()"
   		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)"
		   "\n expected.isStopped          = false"
		   "\n expected.currentConcurrency = 1"
		   "\n expected.numberPending      = nil" // omit for now
		   "\n QEQ(queue, expected)"
   		   "\n lib.aliLuaTest.testUtil.Sleep(10 * 0.1)"
		   "\n expected.numberPending      = 0"
		   "\n expected.currentConcurrency = 0"
		   "\n expected.isBusy             = false"
		   "\n QEQ(queue, expected)"
		   "\n qInfo = queue:GetInfo()"
		   "\n sInfo = qInfo.queueStats:GetInfo()"
		   "\n EQ(sInfo, 'count', 10, 'queue.queueStats.')"
		   "\n");
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
