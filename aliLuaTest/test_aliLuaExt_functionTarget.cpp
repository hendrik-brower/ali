#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using FnTarget   = aliLuaExt::FunctionTarget;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using Util       = aliLuaCore::Util;
  using Pool       = aliSystem::Threading::Pool;
  using Sem        = aliSystem::Threading::Semaphore;

  int sum=0;

  int TestFn(lua_State *L) {
    int a = lua_tointeger(L,1);
    int b = lua_tointeger(L,2);
    lua_pushinteger(L,a+b);
    sum += a + b;
    return 1;
  }
  aliLuaCore::MakeFn PushPair(int a, int b) {
    return [=](lua_State *L) -> int {
      lua_pushinteger(L,a);
      lua_pushinteger(L,b);
      return 2;
    };
  }
}

TEST(aliLuaExtFunctionTarget, general) {
  Future::Ptr       fPtr;
  const std::string fnName = "TestFn";
  FnTarget          fnTarget(fnName);
  int               iSum   = sum;
  int               a1     = 1;
  int               b1     = 7;
  int               a2     = 121;
  int               b2     = 363;
  Pool::Ptr         pool   = Pool::Create("function target test",1);
  ExecEngine::Ptr   exec   = ExecEngine::Create("function target test", pool);
  MakeFn            a1b1   = PushPair(a1,b1);
  MakeFn            a2b2   = PushPair(a2,b2);
  ASSERT_STREQ(fnName.c_str(), fnTarget.FnName().c_str());
  exec->Run([=](lua_State *L) -> int {
      lua_pushcfunction(L,&TestFn);
      lua_setglobal(L, fnName.c_str());
      return 0;
    });
  fnTarget.Run(exec, fPtr, a1b1);
  fPtr = Future::Create();
  fnTarget.Run(exec, fPtr, a2b2);
  aliLuaTest::Util::Wait(exec, fPtr);
  //
  // verify results
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_EQ(sum-iSum, a1+b1+a2+b2);
  lua_State *L = luaL_newstate();
  MakeFn fVal = fPtr->GetValue();
  ASSERT_TRUE(fVal);
  int rtn = fVal(L);
  ASSERT_EQ(rtn,2); // expecting true, a2+b2
  ASSERT_TRUE(lua_toboolean(L,-2));
  ASSERT_EQ(a2+b2, lua_tointeger(L,-1));
  lua_close(L);
}

TEST(aliLuaExtFunctionTarget, luaAPI) {
  int               iSum = sum;
  Future::Ptr       fPtr = Future::Create();
  Pool::Ptr         pool = Pool::Create("function target test",1);
  ExecEngine::Ptr   exec = ExecEngine::Create("function target test", pool);
  exec->Run([=](lua_State *L) -> int {
      lua_pushcfunction(L,&TestFn);
      lua_setglobal(L, "TestFn");
      Future::OBJ::Make(L, fPtr);
      lua_setglobal(L, "future");
      return 0;
    });
  Util::LoadString(exec,
		   "-- function target test"
		   "\n local fnTarget = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'functionTarget',"
		   "\n    fnName     = 'TestFn',"
		   "\n }"
		   "\n local options = {"
		   "\n    exec = lib.aliLua.exec.GetEngine(),"
		   "\n }"
		   "\n fnTarget:Run(options, 43, 15)"
		   "\n options.future = future"
		   "\n fnTarget:Run(options, 99, 31)"
		   );
  aliLuaTest::Util::Wait(exec, fPtr);
  //
  // verify results
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_EQ(sum-iSum, 43+15+99+31);
  lua_State *L = luaL_newstate();
  MakeFn fVal = fPtr->GetValue();
  ASSERT_TRUE(fVal);
  int rtn = fVal(L);
  ASSERT_EQ(rtn,2); // expecting true, a2+b2
  ASSERT_TRUE(lua_toboolean(L,-2));
  ASSERT_EQ(99+31, lua_tointeger(L,-1));
  lua_close(L);
}
