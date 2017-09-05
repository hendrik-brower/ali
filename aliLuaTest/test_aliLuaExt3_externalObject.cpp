#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaExt3.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using Exec           = aliLuaCore::Exec;
  using ExecEngine     = aliLuaExt::ExecEngine;
  using ExternalMT     = aliLuaExt3::ExternalMT;
  using ExternalObject = aliLuaExt3::ExternalObject;
  using Future         = aliLuaCore::Future;
  using MakeFn         = aliLuaCore::MakeFn;
  using Util           = aliLuaCore::Util;
  using BPtr           = aliLuaTest::Util::BPtr;
  using TestUtil       = aliLuaTest::Util;
  using Pool           = aliSystem::Threading::Pool;
  
  const std::string engineName = "extObjEngine";
  const std::string mtName     = "extObjectTestMT";
  const std::string script =
    "-- test ExternalMT"
    "\n local args = nil"
    "\n function Create(...)"
    "\n    args = {...}"
    "\n    return 'created'"
    "\n end"
    "\n function Destroy(...)"
    "\n end"
    "\n function Fn1(arg1, arg2)"
    "\n    return arg1+arg2"
    "\n end"
    "\n function Fn2(...)"
    "\n    return 'fn2'"
    "\n end"
    ;
  const std::string createFn      = "Create";
  const std::string onDestroyFn   = "Destroy";
  ExternalMT::SSet functions;
  ExternalMT::Ptr mt;
  
  void Init() {
    functions.insert("Fn1");
    functions.insert("Fn2");
    mt = ExternalMT::Create(mtName,
			    script,
			    createFn,
			    onDestroyFn,
			    functions);
  }
  void Fini() {
  }

}

void RegisterInitFini_ExternalObjectTest(aliSystem::ComponentRegistry &cr) {
  aliSystem::Component::Ptr ptr = cr.Register("test_aliLuaExt3ExternalObj", Init, Fini);
  ptr->AddDependency("aliLuaCore");
  ptr->AddDependency("aliLuaExt");
  ptr->AddDependency("aliLuaExt2");
}

TEST(aliLuaExtExternalObject, general) {
  Pool::Ptr           pool      = Pool::Create("external mt pool", 1);
  Future::Ptr         loadRes   = Future::Create();
  Future::Ptr         createRes = Future::Create();
  MakeFn              args      = [=](lua_State *L) {
    lua_checkstack(L,2);
    lua_pushnumber(L, 3);
    lua_pushnumber(L, 5);
    return 2;
  };
  ExternalObject::Ptr obj  = ExternalObject::Create(mt,
						    engineName,
						    pool,
						    loadRes,
						    createRes,
						    args);
  Exec::Ptr           exec = obj->GetExec();
  ExternalMT::Ptr     oMT  = obj->EMTPtr();
  ASSERT_TRUE(exec);
  ASSERT_STREQ(exec->Name().c_str(), engineName.c_str());
  ASSERT_EQ(oMT.get(), mt.get());
  TestUtil::Wait(exec, loadRes);
  ASSERT_TRUE(loadRes->IsSet());
  ASSERT_FALSE(loadRes->IsError()) << loadRes->GetError();
  TestUtil::Wait(exec, createRes);
  ASSERT_TRUE(createRes->IsSet());
  ASSERT_FALSE(createRes->IsError()) << createRes->GetError();
  MakeFn rtn = createRes->GetValue();
  BPtr   rtnMatch(new bool(false));
  Future::Ptr fPtr = Future::Create();
  Util::Run(exec, fPtr, [=](lua_State *L) {
      rtn(L);
      if (lua_isboolean(L,1) && lua_toboolean(L,1)) {
	const char *str = lua_tostring(L,2);
	*rtnMatch  = str && lua_gettop(L)==2 && strcmp(str, "created")==0;
      }
      return 0;
    });
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  ASSERT_TRUE(*rtnMatch);

  fPtr = Future::Create();
  obj->RunFunction("Fn1", fPtr, args);
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  
  rtn  = fPtr->GetValue();
  fPtr = Future::Create();
  BPtr fn1ReturnMatch(new bool(false));
  Util::Run(exec, fPtr, [=](lua_State *L) {
      rtn(L);
      if (lua_isboolean(L,1) && lua_toboolean(L,1)) {
	int val = lua_tointeger(L,2);
	*fn1ReturnMatch = val==8;
      }
      return 0;
    });
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
  ASSERT_TRUE(*fn1ReturnMatch);
}

TEST(aliLuaExtExternalObject, scriptInterface) {
  Pool::Ptr       pool = Pool::Create("external mt pool", 1);
  ExecEngine::Ptr exec = ExecEngine::Create("exec", pool);
  Future::Ptr     fPtr = Future::Create();
  Util::LoadString(exec,fPtr,
		   "-- external object tests"
		   "\n local pool = lib.aliLua.threading.CreatePool {"
		   "\n    name       = 'test pool',"
		   "\n    numThreads = 2,"
		   "\n }"
		   "\n local loadResult   = lib.aliLua.future.Create()"
		   "\n local createResult = lib.aliLua.future.Create()"
		   "\n local obj = lib.aliLua.externalObject.Create({"
		   "\n    mtName       = 'extObjectTestMT',"
		   "\n    engineName   = 'external object test engine',"
		   "\n    loadResult   = loadResult,"
		   "\n    createResult = createResult,"
		   "\n    threadPool   = pool,"
		   "\n }, 'arg1', 'arg2')"
		   "\n assert(obj, 'did not retrieve the external mt')"
		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)"
		   "\n assert(createResult:IsSet(), 'createResult is not set')"
		   "\n local rtn = {createResult:GetValue()}"
		   "\n assert(rtn[1]==true,      'rtn[1] is not true')"
		   "\n assert(rtn[2]=='created', 'rtn[2] is not created')"
		   "\n local info = obj:GetInfo()"
		   "\n assert(info.exec,              'exec          undefined')"
		   "\n assert(info.mt,                'mt            undefined')"
		   "\n assert(info.mt.mtName,         'mtName        undefined')"
		   "\n assert(info.mt.script,         'script        undefined')"
		   "\n assert(info.mt.createFn,       'createFn      undefined')"
		   "\n assert(info.mt.onDestroyFn,    'onDestroyFn   undefined')"
		   "\n local callResult = lib.aliLua.future.Create()"
		   "\n obj:Call( {"
		   "\n    fnName     = 'Fn1',"
		   "\n    callResult = callResult,"
		   "\n }, 3, 6)"
		   "\n lib.aliLuaTest.testUtil.Sleep(0.1)"
		   "\n assert(callResult:IsSet(), 'Fn1 did not return (yet?)')"
		   "\n rtn = {callResult:GetValue()}"
		   "\n assert(rtn[1]==true, 'call to Fn1 failed')"
		   "\n assert(rtn[2]==9,    'call to Fn1 did not retrun the expected value')"
		   "");
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

