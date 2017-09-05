#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using ExecPool   = aliLuaExt::ExecPool;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using MTU        = aliLuaCore::MakeTableUtil;
  using StackGuard = aliLuaCore::StackGuard;
  using Util       = aliLuaCore::Util;
  using Values     = aliLuaCore::Values;
  using BPtr       = aliLuaTest::Util::BPtr;
  using IPtr       = aliLuaTest::Util::IPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
  using Time       = aliSystem::Time;
}

TEST(aliLuaExtExecPool, basics) {
  const std::string name   = "exec pool test";
  const std::string added  = "added exec";
  Pool::Ptr         pool   = Pool::Create(name, 4);
  ExecPool::Ptr     ePool  = ExecPool::Create(name, 2, pool);
  ExecEngine::Ptr   engine = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr   = Future::Create();
  ExecPool::ExecVec eVec;
  ePool->Register(ExecEngine::Create(added,pool));
  ASSERT_STREQ(name.c_str(), ePool->Name().c_str());
  ASSERT_EQ(ePool->GetExec().get(), ePool.get());
  ePool->GetRegistered(eVec);
  ASSERT_EQ(eVec.size(), 3u);

  
  Util::Run(engine, fPtr, [=](lua_State *L) {
      StackGuard g(L,10);
      MakeFn simple1 = ExecPool::GetInfoSimple(ePool);
      MakeFn simple2 = ExecPool::GetInfo(true,   ePool);
      MakeFn detail1 = ExecPool::GetInfoDetailed(ePool);
      MakeFn detail2 = ExecPool::GetInfo(false,  ePool);
      simple1(L);
      simple2(L);
      detail1(L);
      detail2(L);
      lua_setglobal(L, "detailed2");
      lua_setglobal(L, "detailed1");
      lua_setglobal(L, "simple2");
      lua_setglobal(L, "simple1");
      return 0;
    });
  TestUtil::Wait(engine, fPtr);
  fPtr = Future::Create();
  Util::LoadString(engine, fPtr,
		   "-- exec pool testing"
		   "\n function CHK(tblName, field, val)"
		   "\n    tbl = _ENV[tblName]"
		   "\n    assert(type(tbl)=='table', 'bad table '..tblName)"
		   "\n    if val~=nil then"
		   "\n       assert(tbl[field]==val, 'bad value for '..field..' in '..tblName)"
		   "\n    else"
		   "\n       assert(tbl[field], 'value not present for '..field..' in '..tblName)"
		   "\n    end"
		   "\n end"
		   "\n CHK('simple1', 'numPending', 0)"
		   "\n CHK('simple1', 'name',       'exec pool test')"
		   "\n CHK('simple1', 'execType',   'execPool')"
		   "\n CHK('simple1', 'isBusy',     false)"
		   "\n CHK('simple1', 'pool')"
		   "\n CHK('simple2', 'numPending', 0)"
		   "\n CHK('simple2', 'name',       'exec pool test')"
		   "\n CHK('simple2', 'execType',   'execPool')"
		   "\n CHK('simple2', 'isBusy',     false)"
		   "\n CHK('simple2', 'pool')"
		   "\n CHK('detailed1', 'numPending', 0)"
		   "\n CHK('detailed1', 'name',       'exec pool test')"
		   "\n CHK('detailed1', 'execType',   'execPool')"
		   "\n CHK('detailed1', 'isBusy',     false)"
		   "\n CHK('detailed1', 'pool')"
		   "\n CHK('detailed1', 'execItems')"
		   "\n CHK('detailed2', 'numPending', 0)"
		   "\n CHK('detailed2', 'name',       'exec pool test')"
		   "\n CHK('detailed2', 'execType',   'execPool')"
		   "\n CHK('detailed2', 'isBusy',     false)"
		   "\n CHK('detailed2', 'pool')"
		   "\n CHK('detailed2', 'execItems')"
		   "\n for _,tblName in ipairs { 'detailed1', 'detailed2' } do"
		   "\n    local found = false"
		   "\n    local tbl   = _ENV[tblName]"
		   "\n    assert(type(tbl)=='table', 'table undefined: '..tblName)"
		   "\n    local execItems = tbl.execItems"
		   "\n    assert(type(execItems)=='table', 'invalid: '..tblName..'.execItems')"
		   "\n    for i=1,3 do"
		   "\n       local key = tblName..'.execItems['..tostring(i)..'].execInfo'"
		   "\n       local execItem = execItems[i].execInfo"
		   "\n       assert(type(execItem)=='table', 'invalid: '..key..'.execItems')"
		   "\n       assert(execItem.name, 'no name for '..key)"
		   "\n       assert(execItem.exec, 'no exec for '..key)"
		   "\n       found = found or execItem.name=='added exec'"
		   "\n    end"
		   "\n    assert(found, 'failed to find the manually added execEngine')"
		   "\n end"
		   "\n");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  size_t sz = 3;
  for (ExecPool::ExecVec::iterator it=eVec.begin(); it!=eVec.end(); ++it) {
    Exec::Ptr ptr = it->lock();
    ePool->Unregister(ptr);
    ExecPool::ExecVec eVec2;
    ePool->GetRegistered(eVec2);
    ASSERT_EQ(eVec2.size(),--sz);
  }
}

TEST(aliLuaExtExecPool, listener) {
  const std::string name      = "my pool";
  Pool::Ptr         pool      = Pool::Create("engine pool", 2);
  ExecPool::Ptr     engine    = ExecPool::Create(name, 2, pool);
  double            delay     = 0.25;
  int               numSleeps = 10;
  BPtr              isEqual(new bool(false));
  IPtr              iPtr(new int(0));

  //
  // 1) let engines intiialize
  // 2) define an onIdle handler
  usleep(250*1000); 
  ExecEngine::Listeners::Register(engine->OnIdle(), "testListerer", [=](const Exec::WPtr&wPtr) {
      ++(*iPtr);
      Exec::Ptr ptr = wPtr.lock();
      *isEqual = ptr.get()==engine.get();
    }, false);
  for (int i=0;i<numSleeps;++i) {
    Util::RunFn(engine, "lib.aliLuaTest.testUtil.Sleep", Values::GetMakeDoubleFn(delay));
  }
  usleep(delay*1000*1000*(1+numSleeps)/2);
  ASSERT_TRUE(*isEqual) << "passed engine should be the execPool";
  ASSERT_EQ(*iPtr, 1);
  usleep(500*1000);
  ASSERT_EQ(*iPtr, 1) << "shouldn't see more than on onIdle signal";
}

TEST(aliLuaExtExecPool, scriptInterface) {
  std::string name = "myExec";
  Pool::Ptr   pool = Pool::Create(name, 1);
  Exec::Ptr   exec = ExecEngine::Create(name,pool);
  Future::Ptr fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
  		   "-- execPool script interface testing"
		   "\n function CHK(tbl, val, ...)"
		   "\n    local path = ''"
		   "\n    local tblValue = tbl"
		   "\n    for _, key in ipairs {...} do"
		   "\n       assert(tblValue, 'table undefined for '..path)"
		   "\n       assert(type(tblValue)=='table', 'expecting a table for info'..path)"
		   "\n       if type(key)=='number' then"
		   "\n          path = path ..'['..key..']'"
		   "\n       else"
		   "\n          path = path .. '.' .. key"
		   "\n       end"
		   "\n       tblValue = tblValue[key]"
		   "\n    end"
		   "\n    if val=='any' then"
		   "\n        assert(tblValue, 'info'..path..' is not defined')"
		   "\n    elseif val==nil then"
		   "\n        assert(not tblValue, 'expected nil for info'..path)"
		   "\n    else"
		   "\n        assert(tblValue==val, 'info'..path..' is not defined')"
		   "\n    end"
		   "\n end"
  		   "\n pool = lib.aliLua.threading.CreatePool {"
  		   "\n     name       = 'test pool',"
  		   "\n     numThreads = 2,"
  		   "\n }"
  		   "\n execPool = lib.aliLua.exec.CreatePool {"
  		   "\n     name       = 'myExecPool',"
  		   "\n     numEngines = 2,"
		   "\n     threadPool = pool,"
  		   "\n }"
  		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)" // delay for pool and ep init for isBusy
  		   "\n info = execPool:GetInfo()"
		   "\n CHK(info, 'execPool',   'execInfo', 'execType')"
		   "\n CHK(info, 'myExecPool', 'execInfo', 'name')"
		   "\n CHK(info, 'any',        'execInfo', 'stats')"
		   "\n CHK(info, 'any',        'execInfo', 'exec')"
		   "\n CHK(info, 'execPool',   'poolInfo', 'execType')"
		   "\n CHK(info, 'myExecPool', 'poolInfo', 'name')"
		   "\n CHK(info, 'any',        'poolInfo', 'pool')"
		   "\n CHK(info, 0,            'poolInfo', 'numPending')"
		   "\n CHK(info, false,        'poolInfo', 'isBusy')"
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 1, 'execInfo', 'name')"
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 2, 'execInfo', 'name')"
		   "\n CHK(info, 'execEngine', 'poolInfo', 'execItems', 1, 'execInfo', 'execType')" // could change
		   "\n CHK(info, 'execEngine', 'poolInfo', 'execItems', 2, 'execInfo', 'execType')" // could change
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 1, 'execInfo', 'exec')"
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 2, 'execInfo', 'exec')"
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 1, 'execInfo', 'stats')"
		   "\n CHK(info, 'any',        'poolInfo', 'execItems', 2, 'execInfo', 'stats')"
		   "\n "
		   "\n exec = info.poolInfo.execItems[1].execInfo.exec"
		   "\n exec = exec:ToStrong()" // it will be released when unregistered otherwise
		   "\n execPool:Unregister(exec)"
		   "\n info = execPool:GetInfo()"
		   "\n assert(#info.poolInfo.execItems==1, 'failed to unregister an exec')"
		   "\n execPool:Register(exec)"
		   "\n info = execPool:GetInfo()"
		   "\n --assert(#info.poolInfo.execItems==2, 'failed to register an exec')"
		   "\n "
  		   "\n");
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}


