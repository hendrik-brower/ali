#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using ObjTarget  = aliLuaExt::ObjectTarget;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using Util       = aliLuaCore::Util;
  using Pool       = aliSystem::Threading::Pool;
  using Sem        = aliSystem::Threading::Semaphore;
  using Values     = aliLuaCore::Values;

  struct Obj {
    int total;
    int last;
    Obj() : total(0), last(0) {}
  };
  using OBJ = aliLuaCore::StaticObject<Obj>;
  
  int TestFn(lua_State *L) {
    OBJ::TPtr obj = OBJ::Get(L,1,false);
    int a = lua_tonumber(L,2);
    int b = lua_tonumber(L,3);
    obj->total = obj->total + a + b;
    obj->last  = a + b;
    Values::MakeInteger(L, obj->last);
    return 1;
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("object target MT");
    mtMap->Add("TestFn", TestFn);
    OBJ::Init("aliLuaExt::ObjectTarget::testing", mtMap, true);
    aliLuaCore::Module::Register("load aliLuaExt::ObjectTarget::testing functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       OBJ::Register(ePtr);
			     });
  }
  void Fini() {
    OBJ::Fini();
  }

  aliLuaCore::MakeFn PushPair(int a, int b) {
    return [=](lua_State *L) -> int {
      lua_pushinteger(L,a);
      lua_pushinteger(L,b);
      return 2;
    };
  }
}

void RegisterInitFini_ObjectTargetTest(aliSystem::ComponentRegistry &cr) {
  aliSystem::Component::Ptr ptr = cr.Register("test_aliLuaExtObjectTarget", Init, Fini);
  ptr->AddDependency("aliLuaCore");
}


TEST(aliLuaExtObjectTarget, general) {
  Future::Ptr       fPtr;
  const std::string fnName = "TestFn";
  OBJ::TPtr         objPtr(new Obj);
  ObjTarget         objTarget(fnName, OBJ::GetMT(), [=](lua_State *L) {
      return OBJ::Make(L,objPtr);
    });
  int               a1     = 1;
  int               b1     = 7;
  int               a2     = 121;
  int               b2     = 363;
  Pool::Ptr         pool   = Pool::Create("function target test",1);
  ExecEngine::Ptr   exec   = ExecEngine::Create("function target test", pool);
  MakeFn            a1b1   = PushPair(a1,b1);
  MakeFn            a2b2   = PushPair(a2,b2);
  ASSERT_STREQ(fnName.c_str(), objTarget.FnName().c_str());
  ASSERT_EQ(OBJ::GetMT().get(), objTarget.GetMT().get());
  objTarget.Run(exec, fPtr, a1b1);
  fPtr = Future::Create();
  objTarget.Run(exec, fPtr, a2b2);
  aliLuaTest::Util::Wait(exec, fPtr);
  //
  // verify results
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_EQ(objPtr->total, a1+b1+a2+b2);
  ASSERT_EQ(objPtr->last,  a2+b2);
  lua_State *L = luaL_newstate();
  MakeFn fVal = fPtr->GetValue();
  ASSERT_TRUE(fVal);
  int rtn = fVal(L);
  ASSERT_EQ(rtn,2); // expecting true, a2+b2
  ASSERT_TRUE(lua_toboolean(L,-2));
  ASSERT_EQ(a2+b2, lua_tointeger(L,-1));
  lua_close(L);
}

TEST(aliLuaExtObjectTarget, luaAPI) {
  OBJ::TPtr         objPtr(new Obj);
  Future::Ptr       fPtr = Future::Create();
  Pool::Ptr         pool = Pool::Create("function target test",1);
  ExecEngine::Ptr   exec = ExecEngine::Create("function target test", pool);
  exec->Run([=](lua_State *L) -> int {
      OBJ::Make(L,objPtr);
      lua_setglobal(L, "object");
      Future::OBJ::Make(L, fPtr);
      lua_setglobal(L, "future");
      return 0;
    });
  Util::LoadString(exec,
		   "-- object target test - luaAPI"
		   "\n local objTarget = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'objectTarget',"
		   "\n    object     = object,"
		   "\n    fnName     = 'TestFn',"
		   "\n }"
		   "\n local options = {"
		   "\n    exec = lib.aliLua.exec.GetEngine(),"
		   "\n }"
		   "\n objTarget:Run(options, 43, 15)"
		   "\n options.future = future"
		   "\n objTarget:Run(options, 99, 31)"
		   );
  aliLuaTest::Util::Wait(exec, fPtr);
  //
  // verify results
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_EQ(objPtr->total, 43+15+99+31);
  ASSERT_EQ(objPtr->last,        99+31);
  lua_State *L = luaL_newstate();
  MakeFn fVal = fPtr->GetValue();
  ASSERT_TRUE(fVal);
  int rtn = fVal(L);
  ASSERT_EQ(rtn,2); // expecting true, a2+b2
  ASSERT_TRUE(lua_toboolean(L,-2));
  ASSERT_EQ(99+31, lua_tointeger(L,-1));
  lua_close(L);
}

