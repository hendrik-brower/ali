#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <fstream>

namespace {
  using Exec        = aliLuaCore::Exec;
  using ExecEngine  = aliLuaExt::ExecEngine;
  using Functions   = aliLuaCore::Functions;
  using FunctionMap = aliLuaCore::FunctionMap;
  using Future      = aliLuaCore::Future;
  using MakeFn      = aliLuaCore::MakeFn;
  using StackGuard  = aliLuaCore::StackGuard;
  using MTU         = aliLuaCore::MakeTableUtil;
  using Util        = aliLuaCore::Util;
  using Values      = aliLuaCore::Values;
  
  using Pool        = aliSystem::Threading::Pool;
  using Sem         = aliSystem::Threading::Semaphore;
  using BPtr        = aliLuaTest::Util::BPtr;
  using IPtr        = aliLuaTest::Util::IPtr;

  namespace locals {
    IPtr runCount;
  }
  struct aliLuaCoreUtil : testing::Test {
    Exec::Ptr exec;
    Pool::Ptr pool;
    void SetUp() {
      locals::runCount.reset(new int(0));
      pool = Pool::Create("actionTestThreads",2);
      exec = ExecEngine::Create("actionTestEngine", pool);
    }
    void TearDown() {
      locals::runCount.reset();
      exec.reset();
      pool.reset();
    }
  };

  int IncRunCount(lua_State *L) {
    ++(*locals::runCount);
    lua_pushinteger(L,*locals::runCount);
    return 1;
  }
  int GetValue(const Future::Ptr &fPtr) {
    int rtn = 0;
    if (fPtr && fPtr->IsSet()) {
      lua_State *L = luaL_newstate();
      MakeFn makeFn = fPtr->GetValue();
      makeFn(L);
      rtn = lua_tointeger(L,-1);
      lua_close(L);
    }
    return rtn;
  }

  Future::Ptr VerifyFn(const Exec::Ptr &exec,
		       const std::string fnName,
		       const int iVal) {
    Future::Ptr fPtr = Future::Create();
    Util::Run(exec, fPtr, [=](lua_State*L) {
	lua_checkstack(L,10);
	int type = lua_getglobal(L,fnName.c_str());
	lua_pop(L,lua_gettop(L));
	if (type==LUA_TFUNCTION) {
	  lua_pushinteger(L,iVal);
	} else {
	  lua_pushinteger(L,0);
	}
	return 1;
      });
    return fPtr;
  }

}

TEST_F(aliLuaCoreUtil, RC) {
  std::string ok  = Util::RCStr(LUA_OK);
  std::string err = Util::RCStr(LUA_ERRRUN);
  ASSERT_STREQ(ok.c_str(),  "LUA_OK");
  ASSERT_STRNE(err.c_str(), "LUA_OK");
}
TEST_F(aliLuaCoreUtil, run) {
  //
  // w/o future
  BPtr didRun(new bool(false));
  int  iVal = 2341;
  Util::Run(exec, [=](lua_State *) {
      *didRun = true;
      return 0;
    });
  aliLuaTest::Util::Wait(exec);
  ASSERT_TRUE(*didRun);
  //
  // w/future
  Future::Ptr fPtr = Future::Create();
  Util::Run(exec, fPtr, Values::GetMakeIntegerFn(iVal));
  aliLuaTest::Util::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  lua_State *L = luaL_newstate();
  fPtr->GetValue()(L);
  ASSERT_EQ(iVal, lua_tointeger(L,-1));
  lua_close(L);
}

TEST_F(aliLuaCoreUtil, runFn) {
  // setup functions
  Util::Run(exec, [=](lua_State*L) {
      lua_pushcfunction(L,IncRunCount);
      lua_setglobal(L, "IncRunCount");
      return 0;
    });
  Util::LoadString(exec,
		   " function IncRunCountN(n)"
		   "   local rtn = 0"
		   "   n=tonumber(n) or 0"
		   "   for i=1,n do"
		   "      rtn = rtn + IncRunCount()"
		   "   end"
		   "   return rtn"
		   " end"
		   " X = {"
		   "   Y = {"
		   "     IncRunCount  = IncRunCount,"
                   "     IncRunCountN = IncRunCountN,"
		   "   },"
		   " }"
		   );
  // run no arguments
  *locals::runCount = 0;
  Util::RunFn(exec, "IncRunCount");
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,1);
  // run with SVec args
  *locals::runCount = 0;
  Util::SVec sVec;
  sVec.push_back("5");
  Util::RunFn(exec,"IncRunCountN", sVec);
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,5);
  // run with makefn args
  *locals::runCount = 0;
  Util::RunFn(exec, "IncRunCountN", Values::GetMakeIntegerFn(3));
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,3);

  // w/future - run no arguments
  *locals::runCount = 0;
  Future::Ptr fPtr = Future::Create();
  Util::RunFn(exec, fPtr, "IncRunCount");
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_EQ(*locals::runCount,1);
  ASSERT_EQ(1, GetValue(fPtr));
  // w/future run with SVec args
  *locals::runCount = 0;
  fPtr = Future::Create();
  sVec.clear();
  sVec.push_back("5");
  Util::RunFn(exec, fPtr, "IncRunCountN", sVec);
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,5);
  ASSERT_EQ(15, GetValue(fPtr));
  // w/future run with makefn args
  *locals::runCount = 0;
  fPtr = Future::Create();
  Util::RunFn(exec, fPtr, "IncRunCountN", Values::GetMakeIntegerFn(3));
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,3);
  ASSERT_EQ(6,GetValue(fPtr));

  // ****************************************************************************************
  // calling a function in a sub table

  // run no arguments
  *locals::runCount = 0;
  Util::RunFn(exec, "X.Y.IncRunCount");
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,1);
  // run with SVec args
  *locals::runCount = 0;
  sVec.clear();
  sVec.push_back("5");
  Util::RunFn(exec,"X.Y.IncRunCountN", sVec);
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,5);
  // run with makefn args
  *locals::runCount = 0;
  Util::RunFn(exec, "X.Y.IncRunCountN", Values::GetMakeIntegerFn(3));
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,3);

  // w/future - run no arguments
  *locals::runCount = 0;
  fPtr = Future::Create();
  Util::RunFn(exec, fPtr, "X.Y.IncRunCount");
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_EQ(*locals::runCount,1);
  ASSERT_EQ(1, GetValue(fPtr));
  // w/future run with SVec args
  *locals::runCount = 0;
  fPtr = Future::Create();
  sVec.clear();
  sVec.push_back("5");
  Util::RunFn(exec, fPtr, "X.Y.IncRunCountN", sVec);
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,5);
  ASSERT_EQ(15, GetValue(fPtr));
  // w/future run with makefn args
  *locals::runCount = 0;
  fPtr = Future::Create();
  Util::RunFn(exec, fPtr, "X.Y.IncRunCountN", Values::GetMakeIntegerFn(3));
  aliLuaTest::Util::Wait(exec);
  ASSERT_EQ(*locals::runCount,3);
  ASSERT_EQ(6,GetValue(fPtr));

  
}

TEST_F(aliLuaCoreUtil, runMT) {
  BPtr match(new bool);
  Util::LoadString(exec,
		   " local mt={}"
		   " mt.__index = mt"
		   " mt.__metatable =mt"
		   " mt.XYZ = function(obj, arg)"
		   "             val = 'hello_'..tostring(arg)" // set a global value
		   "             return val"
		   "          end"
		   " obj = setmetatable({}, mt)");
  Future::Ptr fPtr = Future::Create();
  MakeFn obj = [=](lua_State *L) {
    lua_getglobal(L,"obj");
    return 1;
  };
  Util::RunMT(exec, obj, "XYZ", Values::GetMakeStringFn("call1"));
  *match=false;
  Util::Run(exec, fPtr, [=](lua_State *L) {
      std::string r1 = "hello_call1";
      lua_getglobal(L,"val");
      std::string val = Values::GetString(L,-1);
      *match = val == "hello_call1";
      return 0;
    });
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_TRUE(*match);
  fPtr = Future::Create();
  Util::RunMT(exec, fPtr, obj, "XYZ", Values::GetMakeStringFn("call2"));
  aliLuaTest::Util::Wait(exec,fPtr);
  lua_State *L = luaL_newstate();
  ASSERT_TRUE(fPtr->IsSet());
  fPtr->GetValue()(L);
  ASSERT_STREQ("hello_call2", lua_tostring(L,-1));
  lua_close(L);
}

TEST_F(aliLuaCoreUtil, loadString) {
  // w/o future
  Future::Ptr fPtr;
  const int iVal1 = 155;
  const int iVal2 = 3461;
  Util::LoadString(exec, "function X() return 1 end");
  fPtr = VerifyFn(exec, "X", iVal1);
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_EQ(iVal1, GetValue(fPtr));
  // w/future
  std::stringstream ss;
  ss << "return " << iVal2;
  fPtr = Future::Create();
  Util::LoadString(exec, fPtr, ss.str());
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_EQ(iVal2, GetValue(fPtr));
}

TEST_F(aliLuaCoreUtil, loadFile) {
  // w/o future
  std::string file1 = "testScript_1.lua";
  std::string file2 = "testScript_2.lua";
  std::fstream tmpFile(file1.c_str(), std::ios::out);
  tmpFile << "function Y()"
	  << "\n end"
	  << "\n return 625";
  tmpFile.close();
  tmpFile.open(file2.c_str(), std::ios::out);
  tmpFile << "function Z()"
	  << "\n end"
	  << "\n return 1234";
  tmpFile.close();

  const int iVal1 = 1551;
  const int iVal2 = 1234; // from the file aliLua_UtilTest.f2.lua
  const int iVal3 = 3114;
  Util::LoadFile(exec, file1);
  Future::Ptr fPtr1 = VerifyFn(exec, "Y", iVal1);
  aliLuaTest::Util::Wait(exec, fPtr1);
  ASSERT_EQ(iVal1, GetValue(fPtr1));
  // w/future
  std::stringstream ss;
  ss << "return " << iVal2;
  Future::Ptr fPtr2 = Future::Create();
  Util::LoadFile(exec, fPtr2, file2);
  aliLuaTest::Util::Wait(exec, fPtr2);
  ASSERT_EQ(iVal2, GetValue(fPtr2));
  Future::Ptr fPtr3 = VerifyFn(exec, "Z", iVal3);
  aliLuaTest::Util::Wait(exec, fPtr3);
  ASSERT_EQ(iVal3, GetValue(fPtr3));
}

TEST_F(aliLuaCoreUtil, getElement) {
  MTU mtu;
  lua_State *L = luaL_newstate();
  MTU::Ptr sub1     = mtu.CreateSubtable("sub1");
  MTU::Ptr sub1idx1 = sub1->CreateSubtableForIndex(1);
  MTU::Ptr sub1idx2 = sub1->CreateSubtableForIndex(2);
  MTU::Ptr idx1     = mtu.CreateSubtableForIndex(1);
  MTU::Ptr idx1sub1 = idx1->CreateSubtable("idx1sub1");
  MTU::Ptr idx1sub2 = idx1->CreateSubtable("idx1sub2");
  MTU::Ptr idx2     = mtu.CreateSubtableForIndex(2);
  MTU::Ptr idx21    = idx2->CreateSubtableForIndex(1);
  MTU::Ptr idx22    = idx2->CreateSubtableForIndex(2);
  mtu     . SetString("key01", "value01");
  mtu     . SetString("key02", "value02");
  sub1    ->SetString("key03", "value03");
  sub1idx1->SetString("key04", "value04");
  sub1idx2->SetString("key05", "value05");
  idx1    ->SetString("key06", "value06");
  idx1sub1->SetString("key07", "value07");
  idx1sub2->SetString("key08", "value08");
  idx2    ->SetString("key09", "value09");
  idx21   ->SetString("key10", "value10");
  idx22   ->SetString("key11", "value11");
  mtu.SetBoolean("key12", true);
  mtu.SetNumber("key13", 43);
  mtu.SetNumber("key14", 5.234);
  int rc = mtu.Make(L);
  ASSERT_EQ(rc,1) << "Failed to push the test table on the stack";
  lua_setglobal(L,"tbl");
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl.sub1"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl.sub1[1]"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl.sub1[2]"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[1]"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[1].idx1sub1"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[1].idx1sub2"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[2]"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[2][1]"));
  ASSERT_EQ(LUA_TTABLE, Util::GetElement(L, "tbl[2][2]"));

  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl.key01"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl.key02"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl.sub1.key03"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl.sub1[1].key04"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl.sub1[2].key05"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[1].key06"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[1].idx1sub1.key07"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[1].idx1sub2.key08"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[2].key09"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[2][1].key10"));
  ASSERT_EQ(LUA_TSTRING, Util::GetElement(L, "tbl[2][2].key11"));

  ASSERT_EQ(LUA_TBOOLEAN, Util::GetElement(L, "tbl.key12"));
  ASSERT_EQ(LUA_TNUMBER,  Util::GetElement(L, "tbl.key13"));
  ASSERT_EQ(LUA_TNUMBER,  Util::GetElement(L, "tbl.key14"));
  
  lua_close(L);
}

TEST_F(aliLuaCoreUtil, fnMap) {
  Future::Ptr fPtr = Future::Create();
  IPtr xyz(new int(0));
  IPtr abc(new int(0));
  IPtr def(new int(0));
  FunctionMap::Ptr fnMap = FunctionMap::Create("util test functions");
  fnMap->Add("abc", lua_gettop);
  fnMap->Add("def", lua_gettop);
  Util::LoadFnMap(exec, "tbl.XYZ", fnMap);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      *xyz = Util::GetElement(L, "tbl.XYZ");
      *abc = Util::GetElement(L, "tbl.XYZ.abc");
      *def = Util::GetElement(L, "tbl.XYZ.def");
      return 0;
    });
  aliLuaTest::Util::Wait(exec, fPtr);
  ASSERT_EQ(LUA_TTABLE,    *xyz);
  ASSERT_EQ(LUA_TFUNCTION, *abc);
  ASSERT_EQ(LUA_TFUNCTION, *def);
}
TEST_F(aliLuaCoreUtil, registryFunctions) {
  int          iVal = 2351;
  const char  *jVal = "some string";
  lua_State   *L    = luaL_newstate();
  int          iKey = Util::GetRegistryKey();
  int          jKey = Util::GetRegistryKey();
  ASSERT_EQ(iKey, jKey-1);
  lua_checkstack(L,2);
  lua_pushstring(L, jVal);
  lua_pushinteger(L, iVal);
  Util::RegistrySet(L, iKey);
  Util::RegistrySet(L, jKey);

  lua_rawgeti(L, LUA_REGISTRYINDEX, iKey);
  lua_rawgeti(L, LUA_REGISTRYINDEX, jKey);
  ASSERT_EQ(   iVal, lua_tointeger(L,-2));
  ASSERT_STREQ(jVal, lua_tostring(L,-1));
  lua_close(L);
}
