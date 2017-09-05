#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <lua.hpp>

namespace {
  int TestLuaFn(lua_State *) {
    return 0;
  }
  int TestWrapperFn(const aliLuaCore::LuaFn &luaFn, lua_State *L) {
    return luaFn(L);
  }
  int TestMakeFn(lua_State *) {
    return 0;
  }

  aliLuaCore::MakeFnVec TestMakeFnVec() {
    aliLuaCore::MakeFnVec vec;
    return vec;
  }

  aliLuaCore::KVPair TestKVPair() {
    aliLuaCore::KVPair pair(aliLuaCore::Values::GetMakeIntegerFn(1),
			    aliLuaCore::Values::GetMakeDoubleFn(4.52));
    return pair;
  }
  aliLuaCore::KVVec TestKVVec() {
    aliLuaCore::KVVec vec;
    return vec;
  }
  void TestKVFn(const aliLuaCore::MakeFn &, const aliLuaCore::MakeFn &) {
  }
  void AddKVFn(const aliLuaCore::KVFn &) {
  }
}

TEST(aliLuaCoreTypes, general) {
  aliLuaCore::LuaFn     luaFn     = TestLuaFn;
  aliLuaCore::WrapperFn wrapperFn = TestWrapperFn;
  aliLuaCore::MakeFn    makeFn    = TestMakeFn;
  aliLuaCore::MakeFnVec makeFnVec = TestMakeFnVec();
  aliLuaCore::KVPair    kvPair    = TestKVPair();
  aliLuaCore::KVVec     kvVec     = TestKVVec();

  lua_State *L = luaL_newstate();
  luaFn(L);
  wrapperFn(luaFn, L);
  makeFn(L);
  for (aliLuaCore::MakeFnVec::iterator it=makeFnVec.begin();
       it!=makeFnVec.end(); ++it) {
    (*it)(L);
  }
  kvPair.first(L);
  kvPair.second(L);
  lua_close(L);

  AddKVFn(TestKVFn);
  ASSERT_TRUE(true);
}
