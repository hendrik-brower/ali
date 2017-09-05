#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

/// @notes These tests do not verify cross platform or machine
///        verification.  The Serialization/Deserialization library
///        is intended more for illustrative purposes at this time rather
///        than a core component that should be leveraged outside of
///        exploratative work.

namespace {
  using Deserialize = aliLuaCore::Deserialize;
  using Serialize   = aliLuaCore::Serialize;
  using StackGuard  = aliLuaCore::StackGuard;
  using MakeFn      = aliLuaCore::MakeFn;
  using TestUtil    = aliLuaTest::Util;
  using LPtr        = aliLuaTest::Util::LPtr;
  using Ser         = aliSystem::Codec::Serializer;
  using Des         = aliSystem::Codec::Deserializer;
}

TEST(aliLuaCoreSerialize, general) {
  LPtr        lPtr = TestUtil::GetL();
  lua_State  *L    = lPtr.get();
  bool        b1   = true;
  int         i1   = 993;
  double      d1   = 43.5;
  std::string s1   = "some string";
  bool        b2   = false;
  int         i2   = 789;
  double      d2   = 73.45;
  std::string s2   = "another str";
  std::string sub  = "subTable";
  lua_pushboolean(L,b1);
  lua_pushinteger(L,i1);
  lua_pushnumber(L,d1);
  lua_pushstring(L,s1.c_str());
  lua_newtable(L);
  lua_newtable(L);
  lua_pushvalue(L,-1);
  lua_setfield(L,-3,sub.c_str());
  lua_pushboolean(L,b2);
  lua_setfield(L,-2,"b2");
  lua_pushinteger(L,i2);
  lua_setfield(L,-2,"i2");
  lua_pushnumber(L,d2);
  lua_setfield(L,-2,"d2");
  lua_pushstring(L,s2.c_str());
  lua_setfield(L,-2,"s2");
  // stack: b1,i1,d1,s1,tbl.subtable,subtable
  // subtable: b2->b2, i2->i2, d2->d2, s2->s2
  ASSERT_EQ(lua_gettop(L),6);

  if (true) {
    //
    // test serialization -> deserialization of 1 index
    LPtr       l2Ptr = TestUtil::GetL();
    lua_State *L2    = l2Ptr.get();
    std::stringstream out;
    Ser ser(out);
    Des des(out);
    Serialize::Write(L,1,ser);
    Deserialize::ToLua(L2,des);
    ASSERT_EQ(lua_gettop(L2),1);
    ASSERT_TRUE(lua_isboolean(L2,1));
    ASSERT_TRUE(lua_toboolean(L2,1));
  }

  if (true) {
    //
    // test serialization -> deserialization of 4 index
    LPtr       l2Ptr = TestUtil::GetL();
    lua_State *L2    = l2Ptr.get();
    std::stringstream out;
    Ser ser(out);
    Des des(out);
    Serialize::Write(L,1,4,ser);
    Deserialize::ToLua(L2,des);
    ASSERT_EQ(lua_gettop(L2),4);
    ASSERT_TRUE(lua_isboolean(L2,1));
    ASSERT_TRUE(lua_isnumber (L2,2));
    ASSERT_TRUE(lua_isnumber (L2,3));
    ASSERT_TRUE(lua_isstring (L2,4));
    ASSERT_EQ   (lua_toboolean(L2,1),b1);
    ASSERT_EQ(  lua_tointeger(L2,2),i1);
    ASSERT_EQ(  lua_tonumber (L2,3),d1);
    ASSERT_EQ(  lua_tostring (L2,4),s1);
  }
  
  if (true) {
    //
    // test serialization -> deserialization of last indices (& tables)
    LPtr       l2Ptr = TestUtil::GetL();
    lua_State *L2    = l2Ptr.get();
    std::stringstream out;
    Ser ser(out);
    Des des(out);
    Serialize::Write(L,5,10,ser);
    MakeFn fn = Deserialize::GetMakeFn(des);
    fn(L2);
    ASSERT_EQ(lua_gettop(L2),2);
    ASSERT_TRUE(lua_istable(L2,1));
    ASSERT_TRUE(lua_istable(L2,2));
    lua_getfield(L2,1,sub.c_str());
    ASSERT_EQ(lua_gettop(L2),3);
    ASSERT_TRUE(lua_istable(L2,3));
    lua_getfield(L2,2,"b2"); // 4
    lua_getfield(L2,2,"i2"); // 5
    lua_getfield(L2,2,"d2"); // 6
    lua_getfield(L2,2,"s2"); // 7
    lua_getfield(L2,3,"b2"); // 8
    lua_getfield(L2,3,"i2"); // 9
    lua_getfield(L2,3,"d2"); // 10
    lua_getfield(L2,3,"s2"); // 11
    ASSERT_EQ(lua_gettop(L2),11);
    ASSERT_TRUE(lua_isboolean(L2, 4));
    ASSERT_TRUE(lua_isnumber (L2, 5));
    ASSERT_TRUE(lua_isnumber (L2, 6));
    ASSERT_TRUE(lua_isstring (L2, 7));
    ASSERT_TRUE(lua_isboolean(L2, 8));
    ASSERT_TRUE(lua_isnumber (L2, 9));
    ASSERT_TRUE(lua_isnumber (L2,10));
    ASSERT_TRUE(lua_isstring (L2,11));

    ASSERT_EQ   (lua_toboolean(L2, 4),b2);
    ASSERT_EQ(  lua_tointeger (L2, 5),i2);
    ASSERT_EQ(  lua_tonumber  (L2, 6),d2);
    ASSERT_EQ(  lua_tostring  (L2, 7),s2);
    ASSERT_EQ   (lua_toboolean(L2, 8),b2);
    ASSERT_EQ(  lua_tointeger (L2, 9),i2);
    ASSERT_EQ(  lua_tonumber  (L2,10),d2);
    ASSERT_EQ(  lua_tostring  (L2,11),s2);
  }
  
}

