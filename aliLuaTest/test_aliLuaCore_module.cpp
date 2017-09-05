#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using Module     = aliLuaCore::Module;
  using IPtr       = aliLuaTest::Util::IPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaCoreModule, general) {
  int iVal = 93;
  IPtr i(new int(iVal));
  Module::Register("testModule", [=](const Exec::Ptr &) {
      ++(*i);
    });
  Future::Ptr fPtr;
  Pool::Ptr pool = Pool::Create("moduleTestPool", 1);
  ASSERT_EQ(*i, iVal+0);
  ExecEngine::Ptr ePtr = ExecEngine::Create("moduleTestEngine",pool);
  fPtr = Future::Create();
  // Engine initialization will run in the background.
  // Hence the following wait.
  TestUtil::Wait(ePtr, fPtr);
  ASSERT_EQ(*i, iVal+1);
  ePtr = ExecEngine::Create("moduleTestEngine",pool);
  TestUtil::Wait(ePtr, fPtr);
  ASSERT_EQ(*i, iVal+2);
}
