#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using FunctionMap = aliLuaCore::FunctionMap;
  using LuaFn       = aliLuaCore::LuaFn;
  using BPtr        = aliLuaTest::Util::BPtr;
  using IPtr        = aliLuaTest::Util::IPtr;
  using TestUtil    = aliLuaTest::Util;
}


TEST(aliLuaCoreFunctionMap, general) {
  const std::string name = "function map test";
  FunctionMap::Ptr  ptr  = FunctionMap::Create(name);
  IPtr              count(new int(0));
  IPtr              fn1Count(new int(0));
  IPtr              fn2Count(new int(0));
  IPtr              aliasCount(new int(0));
  IPtr              unknownCount(new int(0));
  BPtr              fn1Works(new bool(false));
  BPtr              fn2Works(new bool(false));
  BPtr              fn3Works(new bool(false));
  ASSERT_TRUE(ptr);
  ASSERT_STREQ(name.c_str(), ptr->Name().c_str());
  ASSERT_FALSE(ptr->IsFrozen()) << "newly created map is frozen";
  ptr->ForEach([=](const std::string &, const LuaFn &) -> bool {
      ++(*count);
      return true;
    });
  ASSERT_EQ(*count,0) << "newly creeated map is not empty";
  ptr->Add("fn1", [=](lua_State *) -> int {
      ++(*fn1Count);
      return 1;
    });
  ptr->Add("fn2", [=](lua_State *) -> int {
      ++(*fn2Count);
      return 2;
    });
  ptr->Alias("fn3", "fn1");
  ASSERT_TRUE(ptr->IsDefined("fn3"));
  ptr->Wrap("fn2", [=](const LuaFn &orig, lua_State *L) -> int {
      ++(*aliasCount);
      return 10+orig(L);
    });
  ASSERT_TRUE(ptr->IsDefined("fn1"));
  ASSERT_TRUE(ptr->IsDefined("fn2"));
  ptr->ForEach([=](const std::string &fnName, const LuaFn &fn) -> bool {
      if (fnName=="fn1") {
	*fn1Works = fn(nullptr)==1;
      } else if (fnName=="fn2") {
      } else if (fnName=="fn3") {
      } else {
	++(*unknownCount);
      }
      return true;
    });
  ASSERT_TRUE(*fn1Works);
  ASSERT_EQ(*fn1Count, 1);
  ptr->ForEach([=](const std::string &fnName, const LuaFn &fn) -> bool {
      if (fnName=="fn1") {
      } else if (fnName=="fn2") {
	*fn2Works = fn(nullptr)==12;
      } else if (fnName=="fn3") {
      } else {
	++(*unknownCount);
      }
      return true;
    });
  ASSERT_TRUE(*fn2Works);
  ASSERT_EQ(*fn2Count, 1);
  ASSERT_EQ(*aliasCount, 1);
  ptr->ForEach([=](const std::string &fnName, const LuaFn &fn) -> bool {
      if (fnName=="fn1") {
      } else if (fnName=="fn2") {
      } else if (fnName=="fn3") {
	*fn3Works = fn(nullptr)==1;
      } else {
	++(*unknownCount);
      }
      return true;
    });
  ASSERT_TRUE(*fn3Works);
  ASSERT_EQ(*fn1Count,2);
  ASSERT_EQ(*unknownCount, 0);
}

TEST(aliLuaCoreFunctionMap, frozen) {
  FunctionMap::Ptr  ptr  = FunctionMap::Create("frozen test");
  ASSERT_FALSE(ptr->IsFrozen());
  ptr->Freeze();
  ASSERT_TRUE(ptr->IsFrozen());
  bool addAfterFreezeThrows = TestUtil::DidThrow([=]() {
      ptr->Add("should throw on add", [](lua_State*) -> int {
	  return 0;
	});
    });
  ASSERT_TRUE(addAfterFreezeThrows);
}
