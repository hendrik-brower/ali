#ifndef INCLUDED_ALI_LUA_TEST_EXEC
#define INCLUDED_ALI_LUA_TEST_EXEC

#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>

namespace aliLuaTest {

  struct Exec : testing::Test {
    aliLuaCore::Exec::Ptr           exec;
    aliSystem::Threading::Pool::Ptr pool;
    void SetUp();
    void TearDown();
  };

}

#endif
