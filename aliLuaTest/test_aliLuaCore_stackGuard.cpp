#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaTest_util.hpp>
#include <lua.hpp>

namespace {
  using StackGuard = aliLuaCore::StackGuard;
  using TestUtil   = aliLuaTest::Util;

  struct aliLuaCoreStackGuard : testing::Test {
    lua_State  *L;
    const int   limit = 10;
    void SetUp() {
      L = luaL_newstate();
    }
    void TearDown() {
      lua_close(L);
    }
  };

}



TEST_F(aliLuaCoreStackGuard, general) {
  const int numItems = 20;
  ASSERT_EQ(lua_gettop(L),0);
  if (true) {
    StackGuard g(L, numItems);
    for (int i=0;i<numItems;++i) {
      lua_pushinteger(L,1000+i);
      ASSERT_EQ(lua_gettop(L),i+1) << "first block i=" << i;
    }
    if (true) {
      StackGuard g(L, numItems);
      for (int i=0;i<numItems;++i) {
	lua_pushinteger(L,2000+i);
	ASSERT_EQ(lua_gettop(L),numItems+i+1) << "second block i=" << i;
      }
    }
    ASSERT_EQ(lua_gettop(L),numItems) << "first block should remain";
  }
  ASSERT_EQ(lua_gettop(L),0) << "stack should be empty";
}
TEST_F(aliLuaCoreStackGuard, index) {
  StackGuard g(L, limit);
  for (int i=0;i<limit; ++i) {
    lua_pushinteger(L, 3*i);
  }
  for (int i=0;i<limit;++i) {
    ASSERT_EQ(3*i, lua_tointeger(L,g.Index(i+1)));
  }
}
TEST_F(aliLuaCoreStackGuard, diff) {
  StackGuard g(L, limit);
  for (int i=0;i<limit; ++i) {
    lua_pushinteger(L, i);
    ASSERT_EQ(i+1, g.Diff());
  }
  for (int i=0;i<limit;++i) {
    lua_pop(L,1);
    ASSERT_EQ(limit-i-1, g.Diff());
  }
}
TEST_F(aliLuaCoreStackGuard, clear) {
  StackGuard g(L, limit);
  for (int i=0;i<limit; ++i) {
    lua_pushinteger(L, i);
    ASSERT_EQ(i+1, g.Diff());
  }
  g.Clear();
  ASSERT_EQ(0, g.Diff());
}
TEST_F(aliLuaCoreStackGuard, verify) {
  StackGuard g(L,limit);
  lua_pushinteger(L,1);
  ASSERT_TRUE( TestUtil::DidThrow([g=&g]() {g->Verify(0, "should throw");}));
  ASSERT_FALSE(TestUtil::DidThrow([g=&g]() {g->Verify(1, "should not throw");}));
  ASSERT_TRUE( TestUtil::DidThrow([g=&g]() {g->Verify(2, "should throw");}));
}
