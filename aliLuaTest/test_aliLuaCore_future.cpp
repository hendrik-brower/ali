#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using Util       = aliLuaCore::Util;
  using Values     = aliLuaCore::Values;
  using BPtr       = aliLuaTest::Util::BPtr;
  using IPtr       = aliLuaTest::Util::IPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}


TEST(aliLuaExtFuture, create) {
  TestUtil::LPtr lPtr = TestUtil::GetL();
  MakeFn         makeFn;
  IPtr           iPtr(new int(0));
  Future::Ptr    f1 = Future::Create();
  Future::Ptr    f2 = Future::Create([=](lua_State *) {
      ++(*iPtr);
      return 0;
    });
  ASSERT_TRUE(f1);
  ASSERT_TRUE(f2);
  ASSERT_FALSE(f1->IsSet());
  ASSERT_TRUE(f2->IsSet());
  ASSERT_FALSE(f1->IsError());
  ASSERT_FALSE(f2->IsError());
  makeFn = f1->GetValue();
  ASSERT_FALSE(makeFn);
  makeFn = f2->GetValue();
  ASSERT_TRUE(makeFn);
  ASSERT_EQ(*iPtr,0);
  makeFn(lPtr.get());
  ASSERT_EQ(*iPtr, 1);
}
TEST(aliLuaExtFuture, setValue) {
  IPtr            iPtr(new int(0));
  TestUtil::LPtr  lPtr = TestUtil::GetL();
  lua_State      *L = lPtr.get();
  MakeFn          makeFn;
  Future::Ptr     f1 = Future::Create();
  std::string     str = "some string 1";
  ASSERT_FALSE(f1->IsSet());
  ASSERT_FALSE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_FALSE(makeFn);
  f1->SetValue([=](lua_State *L) {
      lua_pushstring(L, str.c_str());
      ++(*iPtr);
      return 1;
    });
  ASSERT_TRUE(f1->IsSet());
  ASSERT_FALSE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_TRUE(makeFn);
  ASSERT_EQ(*iPtr,0);
  makeFn(lPtr.get());
  ASSERT_EQ(*iPtr,1);
  ASSERT_TRUE(lua_isboolean(L,1));
  ASSERT_TRUE(lua_isstring(L,2));
  ASSERT_TRUE(lua_toboolean(L,1));
  ASSERT_STREQ(str.c_str(), lua_tostring(L,2));
}
TEST(aliLuaExtFuture, setTimeout) {
  TestUtil::LPtr  lPtr = TestUtil::GetL();
  lua_State      *L = lPtr.get();
  MakeFn          makeFn;
  Future::Ptr     f1 = Future::Create();
  ASSERT_FALSE(f1->IsSet());
  ASSERT_FALSE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_FALSE(makeFn);
  f1->SetTimeout();
  ASSERT_TRUE(f1->IsSet());
  ASSERT_TRUE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_TRUE(makeFn);
  makeFn(L);
  ASSERT_TRUE(lua_isboolean(L,1));
  ASSERT_TRUE(lua_isstring(L,2));
  ASSERT_FALSE(lua_toboolean(L,1));
  ASSERT_STREQ("timeout", lua_tostring(L,2)); // enforcing specific value for timeouts
}
TEST(aliLuaExtFuture, setError) {
  MakeFn            makeFn;
  TestUtil::LPtr    lPtr = TestUtil::GetL();
  lua_State        *L    = lPtr.get();
  Future::Ptr       f1   = Future::Create();
  const std::string err  = "some error";
  ASSERT_FALSE(f1->IsSet());
  ASSERT_FALSE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_FALSE(makeFn);
  f1->SetError(err);
  ASSERT_TRUE(f1->IsSet());
  ASSERT_TRUE(f1->IsError());
  makeFn = f1->GetValue();
  ASSERT_TRUE(makeFn);
  makeFn(L);
  ASSERT_TRUE(lua_isboolean(L,1));
  ASSERT_TRUE(lua_isstring(L,2));
  ASSERT_FALSE(lua_toboolean(L,1));
  ASSERT_STREQ(err.c_str(), lua_tostring(L,2));
}
TEST(aliLuaExtFuture, listeners) {
  IPtr iPtr(new int(0));
  BPtr bPtr(new bool(false));
  Future::Ptr f1 = Future::Create();
  Future::Ptr f2 = Future::Create();
  Future::Ptr f3 = Future::Create();
  void *addr = f1.get();
  aliSystem::Listeners<Future*>::Register(f1->OnSet(),
					  "onSet",
					  [=](Future *f) {
					    ++(*iPtr);
					    *bPtr = f==addr;
					  }, false);
  aliSystem::Listeners<Future*>::Register(f2->OnSet(),
					  "onSet",
					  [=](Future *) {
					    ++(*iPtr);
					  }, false);
  aliSystem::Listeners<Future*>::Register(f3->OnSet(),
					  "onSet",
					  [=](Future *) {
					    ++(*iPtr);
					  }, false);
  ASSERT_EQ(*iPtr,0);
  f1->SetValue(Values::MakeNil);
  ASSERT_EQ(*iPtr,1);
  ASSERT_TRUE(*bPtr);
  f2->SetTimeout();
  ASSERT_EQ(*iPtr,2);
  f3->SetError("some error");
  ASSERT_EQ(*iPtr,3);
}
TEST(aliLuaExtFuture, scriptInterface) {
  const std::string name = "future tests";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       f1   = Future::Create();
  Future::Ptr       f2   = Future::Create();
  Util::LoadString(exec, f1,
		   "-- future testing"
		   "\n function onSet(f, arg2, arg3)"
		   "\n   test_isSet  = f:IsSet()"
		   "\n   test_value1 = arg2"
		   "\n   test_value2 = arg3"
		   "\n end"
		   "\n test_future = lib.aliLua.future.Create()"
		   "\n assert(test_future, 'failed to create the future')"
		   "\n local exec   = lib.aliLua.exec.GetEngine()"
		   "\n local target = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'functionTarget',"
		   "\n    fnName     = 'onSet',"
		   "\n }"
		   "\n test_listener = test_future:OnSet({"
		   "\n    target = target,"
		   "\n    exec   =   exec,"
		   "\n }, test_future, 'x', 'y')"
		   "\n assert(not test_future:IsSet(), 'future is already set')"
		   "\n test_future:SetValue(1,3,5,9)"
		   "\n assert(test_future:IsSet(), 'future is not set')"
		   "\n local val = {test_future:GetValue()}"
		   "\n assert(val[1] == true, 'expecting val[1]== true')"
		   "\n assert(val[2] ==    1, 'expecting val[2]==    1')"
		   "\n assert(val[3] ==    3, 'expecting val[3]==    3')"
		   "\n assert(val[4] ==    5, 'expecting val[4]==    5')"
		   "\n assert(val[5] ==    9, 'expecting val[5]==    9')"
		   "\n assert(val[6] ==  nil, 'expecting val[6]==  nil')"
		   );
  TestUtil::Wait(exec,f1);
  ASSERT_TRUE(f1->IsSet());
  ASSERT_FALSE(f1->IsError());
  Util::LoadString(exec,f2,
		   "-- future testing - verification"
		   "\n assert(test_isSet, 'test_isSet is not set')"
		   "\n assert(test_value1=='x', 'bad test_value1')"
		   "\n assert(test_value2=='y', 'bad test_value2')"
		   "");
  TestUtil::Wait(exec,f2);
  ASSERT_TRUE(f2->IsSet());
  ASSERT_FALSE(f2->IsError());
}

