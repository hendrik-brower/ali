#include <gtest/gtest.h>
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using CallTarget = aliLuaCore::CallTarget;
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using MakeFn     = aliLuaCore::MakeFn;
  using MTU        = aliLuaCore::MakeTableUtil;
  using Table      = aliLuaCore::Table;
  using Util       = aliLuaCore::Util;
  using Values     = aliLuaCore::Values;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
  int Echo(lua_State *L) {
    return lua_gettop(L);
  }
  struct TestTarget : public CallTarget {
    using Ptr = std::shared_ptr<TestTarget>;
    static CallTarget::Ptr Create(lua_State *L) {
      Ptr rtn(new TestTarget);
      Table::GetInteger(L, 1, "iVal",   *rtn->iVal,      false);
      Table::GetString (L, 1, "infoKey", rtn->infoKey,   false);
      Table::GetString (L, 1, "infoVal", rtn->infoValue, false);
      return rtn;
    }
    MakeFn GetInfo() {
      MTU tbl;
      tbl.SetString(infoKey, infoValue);
      tbl.SetNumber("iVal", *iVal);
      tbl.SetMakeFn("lastArgs", lastArgs);
      return tbl.GetMakeFn();
    }
    TestTarget()
      : iVal(new int(0)),
	lastArgs(Values::MakeNil) {
    }
    const TestUtil::IPtr &Val() const { return iVal; }
    void Run(const Exec::Ptr &ePtr, const FuturePtr &fPtr, const MakeFn &args) {
      ++(*iVal);
      lastArgs = args;
      if (ePtr) {
	Util::Run(ePtr, fPtr, [=](lua_State *L) {
	    args(L);
	    return Echo(L);
	  });
      }
      if (fPtr) {
	fPtr->SetValue(Values::GetMakeIntegerFn(*iVal));
      }
    }
  private:
    TestUtil::IPtr iVal;
    std::string    infoKey;
    std::string    infoValue;
    MakeFn         lastArgs;
  };


}


TEST(aliLuaCoreCallTarget, run) {
  TestTarget  tt;
  Exec::Ptr   ePtr;
  Future::Ptr fPtr;
  fPtr = Future::Create();
  tt.Run(ePtr, fPtr, Values::MakeNil);
  TestUtil::Wait(ePtr,fPtr);
  ASSERT_EQ(*tt.Val(), 1);
  fPtr = Future::Create();
  tt.Run(ePtr, fPtr, Values::MakeNil);
  TestUtil::Wait(ePtr,fPtr);
  ASSERT_EQ(*tt.Val(), 2);
}
TEST(aliLuaCoreCallTarget, registerUnregister) {
  const std::string name="testTarget";
  CallTarget::Register(name, TestTarget::Create);
  ASSERT_TRUE(TestUtil::DidThrow([=]() {
	CallTarget::Register(name, TestTarget::Create);
      }));
  CallTarget::Unregister(name); 
  ASSERT_FALSE(TestUtil::DidThrow([=]() {
	CallTarget::Register(name, TestTarget::Create);
      }));
  CallTarget::Unregister(name);
}

TEST(aliLuaCoreCallTarget, scriptedGetInfo) {
  const std::string name="testTarget2";
  CallTarget::Register(name, TestTarget::Create);
  Future::Ptr fPtr = Future::Create();
  Pool::Ptr   pool = Pool::Create(name, 1);
  Exec::Ptr   ePtr = ExecEngine::Create(name, pool);
  Util::LoadString(ePtr, fPtr,
		   "-- call target test - scriptedGetInfo"
		   "\n local obj = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'testTarget2',"
		   "\n    iVal       = 43,"
		   "\n    infoKey    = 'ikey',"
		   "\n    infoVal    = 'ival',"
		   "\n }"
		   "\n local info = obj:GetInfo()"
		   "\n local infoKey = next(info)"
		   "\n assert(info.ikey=='ival', 'invalid info key/value')"
		   "\n");
  TestUtil::Wait(ePtr, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST(aliLuaCoreCallTarget, scriptedRun) {
  const std::string name="testTarget3";
  CallTarget::Register(name, TestTarget::Create);
  Future::Ptr fPtr = Future::Create();
  Pool::Ptr   pool = Pool::Create(name, 1);
  Exec::Ptr   ePtr = ExecEngine::Create(name, pool);
  Util::LoadString(ePtr, fPtr,
		   "-- call target test scripted run"
		   "\n local iVal = 43"
		   "\n local obj = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'testTarget3',"
		   "\n    iVal       = iVal,"
		   "\n    infoKey    = 'ikey',"
		   "\n    infoVal    = 'ival',"
		   "\n }"
		   "\n local options = {"
		   "\n    exec = lib.aliLua.exec.GetEngine(),"		   
		   "\n }"
		   "\n obj:Run(options, 'xyz')"
		   "\n local info = obj:GetInfo()"
		   "\n assert(info.iVal==iVal+1, 'iVal is expected to be 44 (43+1)')"
		   "\n assert(info.lastArgs=='xyz', 'invalid lastArgs')"
		   "");
  TestUtil::Wait(ePtr, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}


