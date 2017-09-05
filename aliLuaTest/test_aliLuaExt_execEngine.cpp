#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using MTU        = aliLuaCore::MakeTableUtil;
  using Util       = aliLuaCore::Util;
  using Values     = aliLuaCore::Values;
  using BPtr       = aliLuaTest::Util::BPtr;
  using IPtr       = aliLuaTest::Util::IPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaExtExecEngine, general) {
  const std::string name   = "my engine";
  Pool::Ptr         pool   = Pool::Create("engine pool", 2);
  ExecEngine::Ptr   engine = ExecEngine::Create(name, pool);
  Exec::Ptr         exec   = engine->GetExec();
  Future::Ptr       fPtr   = Future::Create();
  BPtr              gotEngine(new bool(false));
  BPtr              isBusy(new bool(false));
  ASSERT_STREQ(name.c_str(), engine->Name().c_str());
  ASSERT_EQ(engine.get(), exec.get());
  Util::Run(engine, fPtr, [=](lua_State *L) {
      ExecEngine::Ptr ePtr = ExecEngine::GetEngine(L);
      *gotEngine = engine.get()==ePtr.get();
      *isBusy = engine->IsBusy();
      MakeFn info = ePtr->Exec::GetInfo();
      info(L);
      lua_setglobal(L,"info");
      return 0;
    });
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  ASSERT_TRUE(*gotEngine);
  ASSERT_TRUE(*isBusy);
  // At this point the engine should be idle.  Between its creation and the 'Run'
  // just above here, the engine will be loading MT or other initialization defined
  // code, so if we check that its idle up there, it would potentially be a race
  // condition, so we check that the engine is idle here.
  ASSERT_FALSE(engine->IsBusy());
  fPtr = Future::Create();
  Util::LoadString(engine, fPtr,
		   "-- execEngine general testing"
		   "\n assert(type(info)=='table', 'no info table')"
		   "\n assert(type(info.engineInfo)=='table', 'no engine info table')"
		   "\n assert(info.engineInfo.name=='my engine', 'bad name')"
		   "\n assert(info.engineInfo.engine, 'engine field not defined')"
		   "\n assert(info.engineInfo.isBusy==true, 'isBusy is not set')"
		   "\n assert(info.engineInfo.execType=='execEngine',"
		   "\n        'execType is not set correctly')"
		   "\n");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

TEST(aliLuaExtExecEngine, onIdle) {
  const std::string name   = "my engine";
  Pool::Ptr         pool   = Pool::Create("engine pool", 2);
  ExecEngine::Ptr   engine = ExecEngine::Create(name, pool);
  Exec::Ptr         exec   = engine->GetExec();
  Future::Ptr       fPtr   = Future::Create();
  BPtr              isEqual(new bool(false));
  IPtr              iPtr(new int(0));
  Util::Run(engine, fPtr, Values::MakeNothing);
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  ExecEngine::Listeners::Register(engine->OnIdle(), "testListerer", [=](const Exec::WPtr&wPtr) {
      ++(*iPtr);
      Exec::Ptr ptr = wPtr.lock();
      *isEqual = ptr.get()==engine.get();
    }, false);
  for (int i=0;i<10;++i) {
    fPtr = Future::Create();
    Util::Run(engine, fPtr, Values::MakeNothing);
    TestUtil::Wait(engine, fPtr);
    ASSERT_TRUE(fPtr->IsSet());
    ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
    usleep(1000); // There a race condition here that the sleep avoids -- generally works.
    ASSERT_EQ(*iPtr,2*(i+1));
    ASSERT_TRUE(*isEqual);
  }
}

TEST(aliLuaExtExecEngine, scriptInterface) {
  std::string     name   = "myExecEngine";
  Pool::Ptr       pool   = Pool::Create(name, 1);
  ExecEngine::Ptr engine = ExecEngine::Create(name,pool);
  Future::Ptr     fPtr   = Future::Create();
  Util::LoadString(engine, fPtr,
		   "-- execEngine script interface testing"
		   "\n pool = lib.aliLua.threading.CreatePool {"
		   "\n     name       = 'test pool',"
		   "\n     numThreads = 2,"
		   "\n }"
		   "\n e1 = lib.aliLua.exec.GetEngine()"
		   "\n e2 = lib.aliLua.exec.CreateEngine {"
		   "\n         name       = 'myExecEngine2',"
		   "\n         threadPool = pool,"
		   "\n }"
		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)" // delay for pool and e2 init for isBusy
		   "\n i1 = e1:GetInfo()"
		   "\n i2 = e2:GetInfo()"
		   "\n for _, i in ipairs {i1,i2} do"
		   "\n    assert(i.execInfo, 'no execInfo field')"
		   "\n    assert(i.execInfo.execType=='execEngine', 'bad execInfo.execType')"
		   "\n    assert(i.execInfo.stats, 'no stats')"
		   "\n    assert(i.execInfo.exec, 'no exec')"
		   "\n    assert(i.engineInfo, 'no engineInfo field')"
		   "\n    assert(i.engineInfo.engine, 'no engineInfo.engine field')"
		   "\n    assert(i.engineInfo.execType=='execEngine', 'bad engineInfo.execType')"
		   "\n end"
		   "\n assert(i1.engineInfo.numPending==1, 'i1.engineInfo.numPending != 1')"
		   "\n assert(i2.engineInfo.numPending==0, 'i2.engineInfo.numPending != 0')"
		   "\n assert(i1.execInfo.name=='myExecEngine',  'invalid i1.execInfo.name')"
		   "\n assert(i2.execInfo.name=='myExecEngine2', 'invalid i2.execInfo.name')"
		   "\n assert(i1.engineInfo.isBusy==true,  'bad i1.engineInfo.isBusy')"
		   "\n assert(i2.engineInfo.isBusy==false, 'bad i2.engineInfo.isBusy')"
		   "\n");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

TEST(aliLuaExtExecEngine, scriptLoads) {
  std::string     name   = "myExecEngine";
  Pool::Ptr       pool   = Pool::Create(name, 1);
  ExecEngine::Ptr engine = ExecEngine::Create(name,pool);
  Future::Ptr     fPtr   = Future::Create();
  Util::LoadString(engine, fPtr,
		   "-- execEngine scripted load testing"
		   "\n local engine = lib.aliLua.exec.GetEngine()"
		   "\n loadStringResult = lib.aliLua.future.Create()"
		   "\n engine:LoadString {"
		   "\n   str    = 'return \"in script\"',"
		   "\n   future = loadStringResult,"
		   "\n }"
		   "\n loadFileResult = lib.aliLua.future.Create()"
		   "\n local fp = io.open('tmpScript.lua', 'w')"
		   "\n fp:write('return \"in file\"')"
		   "\n fp:close()"
		   "\n engine:LoadFile {"
		   "\n   path   = 'tmpScript.lua',"
		   "\n   future = loadFileResult,"
		   "\n }"
		   "\n");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  fPtr = Future::Create();
  Util::LoadString(engine, fPtr,
		   "-- execEngine script load verify"
		   "\n v1 = {loadStringResult:GetValue()}"
		   "\n v2 = {loadFileResult  :GetValue()}"
		   "\n assert(v1[1], 'failed to load test string')"
		   "\n assert(v2[1], 'failed to load test file')"
		   "\n assert(v1[2]=='in script', 'bad load string return')"
		   "\n assert(v2[2]=='in file',   'bad load file return')"
		   "\n ");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

