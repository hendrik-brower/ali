#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using Item   = aliLuaExt::ExecPoolItem;
  using Future = aliLuaCore::Future;
  using BPtr   = aliLuaTest::Util::BPtr;
}

TEST(aliLuaExtExecPoolItem, general) {
  BPtr        bPtr(new bool(false));
  Future::Ptr fPtr = Future::Create();
  Item        item(fPtr, [=](lua_State*) {
      *bPtr = true;
      return 0;
    });
  ASSERT_EQ(fPtr.get(), item.GetFuture().get());
  item.GetLuaFn()(nullptr);
  ASSERT_TRUE(*bPtr);
}
