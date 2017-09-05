#include <gtest/gtest.h>
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaExt2.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using Future   = aliLuaCore::Future;
  using Target   = aliLuaCore::CallTarget;
  using MakeFn   = aliLuaCore::MakeFn;
  using Util     = aliLuaCore::Util;
  using Values   = aliLuaCore::Values;
  using Action   = aliLuaExt2::Action;
  using DepVec   = aliLuaExt2::Action::DepVec;
  using Engine   = aliLuaExt::ExecEngine;
  using FnTarget = aliLuaExt::FunctionTarget;
  using BPtr     = aliLuaTest::Util::BPtr;
  using TestUtil = aliLuaTest::Util;
  using Pool     = aliSystem::Threading::Pool;
  using Time     = aliSystem::Time;
}

TEST(aliLuaExtAction, general) {
  using DSet = std::set<void*>;
  const std::string name = "action test";
  Pool::Ptr         pool = Pool::Create(name, 1);
  Engine::Ptr       exec = Engine::Create(name, pool);
  Target::Ptr       target(new FnTarget("SetResult"));
  MakeFn            args = [=](lua_State *L) {
    lua_checkstack(L,2);
    lua_pushnumber(L,21);
    lua_pushnumber(L,11);
    return 2;
  };
  Util::LoadString(exec,
		   "-- action test - general"
		   "\n function SetResult(a,b)"
		   "\n    _ENV.a = a"
		   "\n    _ENV.b = b"
		   "\n    return a + b"
		   "\n end");
  DSet              dSet;
  DepVec            dVec;
  Future::Ptr       f1 = Future::Create();
  Future::Ptr       f2 = Future::Create();
  Time::TP          timeout = Time::Now()+std::chrono::seconds(1);
  dVec.push_back(f1);
  dVec.push_back(f2);
  Action::Ptr       act    = Action::Create(name,
					    target,
					    exec,
					    args,
					    timeout,
					    dVec);
  Future::Ptr res = act->GetResult();
  ASSERT_STREQ(name.c_str(), act->GetName().c_str());
  ASSERT_EQ(target.get(), act->GetTarget().get());
  ASSERT_EQ(exec.get(), act->GetExec().get());
  ASSERT_NE(res.get(), nullptr);
  ASSERT_FALSE(res->IsSet());
  ASSERT_FALSE(act->IsReady());
  ASSERT_FALSE(act->HasRun());
  act->ForEachDependency([&](const Future::Ptr &f) {
      dSet.insert(f.get());
    });
  ASSERT_EQ(dSet.size(),2u);
  for (DepVec::iterator it=dVec.begin(); it!=dVec.end(); ++it) {
    dSet.erase(it->get());
  }
  ASSERT_EQ(dSet.size(),0u);
  f1->SetValue(Values::MakeNothing);
  ASSERT_FALSE(res->IsSet());
  ASSERT_FALSE(act->IsReady());
  ASSERT_FALSE(act->HasRun());
  f2->SetValue(Values::MakeNothing);
  usleep(10*1000); // some time to allow the call target to queue & execute
  ASSERT_TRUE(res->IsSet());
  ASSERT_FALSE(res->IsError());
  ASSERT_TRUE(act->IsReady());
  ASSERT_TRUE(act->HasRun());
  f1 = Future::Create();
  Util::LoadString(exec, f1,
		   "-- action test - verify result"
		   "\n assert(_ENV.a==21, 'arg a not passed through')"
		   "\n assert(_ENV.b==11, 'arg b not passed through')"
		   "\n");
  TestUtil::Wait(exec, f1);
  ASSERT_TRUE(f1->IsSet());
  ASSERT_FALSE(f1->IsError()) << f1->GetError();
  f1 = Future::Create();
  BPtr isTrue(new bool(false));
  BPtr isSum(new bool(false));
  Util::Run(exec, f1, [=](lua_State *L) {
      res->GetValue()(L);
      *isTrue = lua_isboolean(L,1) && lua_toboolean(L,1);
      *isSum  = lua_isnumber(L,2) && lua_tonumber(L,2)==32;
      return 0;
    });
  TestUtil::Wait(exec, f1);
  ASSERT_TRUE(f1->IsSet());
  ASSERT_FALSE(f1->IsError()) << f1->GetError();
  ASSERT_TRUE(*isTrue);
  ASSERT_TRUE(*isSum);

  //
  // re-do, but let it timeout
  f1 = Future::Create();
  f2 = Future::Create();
  dVec.clear();
  dVec.push_back(f1);
  dVec.push_back(f2);
  timeout = Time::Now() + std::chrono::milliseconds(10);
  act = Action::Create(name, target, exec, args, timeout, dVec);
  res = act->GetResult();
  ASSERT_FALSE(res->IsSet());
  ASSERT_FALSE(act->IsReady());
  ASSERT_FALSE(act->HasRun());
  usleep(20*1000);
  ASSERT_TRUE(res->IsSet());
  ASSERT_TRUE(res->IsError());
  ASSERT_FALSE(act->IsReady());
  ASSERT_TRUE(act->HasRun());


  //
  // test explicit set timeout
  timeout = Time::Now() + std::chrono::seconds(10);
  act = Action::Create(name, target, exec, args, timeout, dVec);
  res = act->GetResult();
  act->SetTimeout();
  ASSERT_TRUE(res->IsSet());
  ASSERT_STREQ(res->GetError().c_str(),"timeout");

  //
  // test explicit set error
  timeout = Time::Now() + std::chrono::seconds(10);
  act = Action::Create(name, target, exec, args, timeout, dVec);
  res = act->GetResult();
  act->SetError("some error reason");
  ASSERT_TRUE(res->IsSet());
  ASSERT_STREQ(res->GetError().c_str(),"some error reason");
}


TEST(aliLuaExtAction, scriptInterface) {
  Pool::Ptr   pool = Pool::Create("action test pool", 1);
  Engine::Ptr exec = Engine::Create("action test engine", pool);
  Future::Ptr fPtr = Future::Create();

  Util::LoadString(exec, fPtr,
		   "-- test action script interface"
		   "\n local pool = lib.aliLua.threading.CreatePool {"
		   "\n    name       = 'pool',"
		   "\n    numThreads = 1,"
		   "\n }"
		   "\n local exec = lib.aliLua.exec.CreateEngine {"
		   "\n    name       = 'test exec',"
		   "\n    threadPool = pool,"
		   "\n }"
		   "\n exec:LoadString {"
		   "\n    str = 'function TestFn(a,b) _ENV.a=a _ENV.b=b end',"
		   "\n }"
		   "\n local target = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'functionTarget',"
		   "\n    fnName     = 'TestFn',"
		   "\n }"
		   "\n local d1     = lib.aliLua.future.Create()"
		   "\n local act = lib.aliLua.action.Create({"
		   "\n    name         = 'act1',"
		   "\n    target       = target,"
		   "\n    exec         = exec,"
		   "\n    timeoutDelay = 1.0,"
		   "\n    dependencies = {d1},"
		   "\n }, 5, 17)"
		   "\n local info = act:GetInfo()"
		   "\n assert(info.name   =='act1', 'bad name')"
		   "\n assert(info.isReady== false, 'bad isReady')"
		   "\n assert(info.hasRun == false, 'bad hasRun')"
		   "\n assert(info.target,       'missing target')"
		   "\n assert(info.exec,         'missing exec')"
		   "\n assert(info.result,       'missing result')"
		   "\n assert(info.dependencies, 'missing dependencies')"
		   "\n assert(#info.dependencies==1, 'expecting just 1 dependency')"
		   "\n local res = info.result"
		   "\n assert(res:IsSet()==false, 'result is already set')"
		   "\n "
		   "\n -- verify timeout call"
		   "\n act:SetTimeout()"
		   "\n assert(res:IsSet())"
		   "\n local rtn = {res:GetValue()}"
		   "\n assert(rtn[1]==false, 'error not flagged')"
		   "\n assert(rtn[2]=='timeout', 'error is not timeout')"
		   "\n "
		   "\n -- verify set error call"
		   "\n act = lib.aliLua.action.Create({"
		   "\n    name         = 'act1',"
		   "\n    target       = target,"
		   "\n    exec         = exec,"
		   "\n    timeoutDelay = 1.0,"
		   "\n    dependencies = {d1},"
		   "\n }, 5, 17)"
		   "\n res = act:GetInfo().result"
		   "\n act:SetError('my error')"
		   "\n assert(res:IsSet())"
		   "\n rtn = {res:GetValue()}"
		   "\n assert(rtn[1]==false, 'error not flagged')"
		   "\n assert(rtn[2]=='my error', 'error is not my error')"
		   "\n");

  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

    
