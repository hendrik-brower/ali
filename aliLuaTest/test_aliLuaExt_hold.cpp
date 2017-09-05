#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using Util       = aliLuaCore::Util;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaExtHold, luaTest) {
  const static std::string name = "hold test";
  Pool::Ptr       pool = Pool::Create(name, 1);
  ExecEngine::Ptr ePtr = ExecEngine::Create(name, pool);
  Future::Ptr     fPtr = Future::Create();
  Util::LoadString(ePtr, fPtr,
		   "-- hold test script"
		   "\n function Verify(hInfo, file, reason, line)"
		   "\n   assert(hInfo, 'failed to get hold info')"
		   "\n   assert(hInfo.file  ==   file, 'incorrect file name '..tostring(hInfo.file))"
		   "\n   assert(hInfo.reason== reason, 'incorrect reason '   ..tostring(hInfo.reason))"
		   "\n   assert(hInfo.line  ==   line, 'incorrect line '     ..tostring(hInfo.line))"
		   "\n end"
		   "\n function Match(i1, i2)"
		   "\n    local match=false"
		   "\n    if i1.file==i2.file then"
		   "\n       if i1.reason==i2.reason then"
		   "\n          if i1.line==i2.line then"
		   "\n             match=true"
		   "\n          end"
		   "\n       end"
		   "\n    end"
		   "\n    return match"
		   "\n end"
		   "\n local hold1 = lib.aliLua.hold.Create {"
		   "\n    file   = 'X',"
		   "\n    reason = 'Y',"
		   "\n    line   = 153,"
		   "\n }"
		   "\n local hold2 = lib.aliLua.hold.Create {"
		   "\n    file   = 'A',"
		   "\n    reason = 'B',"
		   "\n    line   = 999,"
		   "\n }"
		   "\n assert(hold1, 'failed to create hold1')"
		   "\n assert(hold2, 'failed to create hold2')"
		   "\n Verify(hold1:GetInfo(), 'X', 'Y', 153)"
		   "\n Verify(hold2:GetInfo(), 'A', 'B', 999)"
		   "\n local holds = lib.aliLua.hold.GetHolds()"
		   "\n assert(holds[1] and holds[2], 'there should be at least 2 holds')"
		   "\n local found1 = false"
		   "\n local found2 = false"
		   "\n for i, hold in ipairs(holds) do"
		   "\n    if Match(hold1:GetInfo(), hold:GetInfo()) then"
		   "\n       found1 = true"
		   "\n    end"
		   "\n    if Match(hold2:GetInfo(), hold:GetInfo()) then"
		   "\n       found2 = true"
		   "\n    end"
		   "\n end"
		   "\n assert(found1, 'failed to find hold1')"
		   "\n assert(found2, 'failed to find hold2')"
		   );
  TestUtil::Wait(ePtr, fPtr);
  ASSERT_TRUE(fPtr->IsSet()) << "failed to run test script";
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}
