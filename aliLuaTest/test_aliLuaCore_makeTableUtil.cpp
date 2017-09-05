#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <functional>
#include <lua.hpp>

namespace {
  namespace locals {
    const double dDiff = 0.00000001;
  }
}

TEST(aliLuaCoreMakeTableUtil, general) {
  lua_State *L = luaL_newstate();
  aliLuaCore::StackGuard g(L,0);  
  const int                 i1Val = 9246;
  const double              dVal = 224.9324;
  const bool                bVal = true;
  const int                 iKey = 50;
  const int                 i2Val = 23629;
  const std::string         s1Val = "sValue1";
  const std::string         s2Val = "sValue2";
  const std::string         s3Val = "sValue3";
  const std::string         s4Val = "sValue4";
  const std::string         s5Val = "sValue5";
  const aliLuaCore::MakeFn  keyFn = aliLuaCore::Values::GetMakeIntegerFn(iKey);
  const aliLuaCore::MakeFn  valFn = aliLuaCore::Values::GetMakeIntegerFn(i1Val);
  const aliLuaCore::MakeFn  mkVal = aliLuaCore::Values::GetMakeStringFn(s2Val);
  aliLuaCore::MakeTableUtil tblUtil;  
  tblUtil.Set(keyFn, valFn);
  tblUtil.SetBoolean("bVal",  bVal);
  tblUtil.SetNumber("iVal",  i2Val);
  tblUtil.SetNumber("dVal",  dVal);
  tblUtil.SetString("sVal",  s1Val);
  tblUtil.SetMakeFn("mkVal", mkVal);
  tblUtil.SetBooleanForIndex(1, bVal);
  tblUtil.SetNumberForIndex (2, i2Val);
  tblUtil.SetNumberForIndex (3, dVal);
  tblUtil.SetStringForIndex (4, s3Val);
  tblUtil.SetMakeFnForIndex (5, mkVal);
  tblUtil.CreateSubtable("sub1")->SetString("sub1Key", s4Val);
  tblUtil.CreateSubtableForIndex(6   )->SetString("sub2Key", s5Val);
  aliLuaCore::MakeFn mkTbl = tblUtil.GetMakeFn();

  ASSERT_EQ(g.Diff(),0);
  tblUtil.Make(L);
  ASSERT_EQ(g.Diff(),1);
  mkTbl(L);
  ASSERT_EQ(g.Diff(),2);
  ASSERT_TRUE(lua_istable(L,g.Index(1)));
  ASSERT_TRUE(lua_istable(L,g.Index(1)));

  auto verify = [&](int tblIdx) -> void {
    //
    // extract table contents
    ASSERT_TRUE(lua_istable(L,tblIdx)) << "Expecting a table at " << tblIdx;
    aliLuaCore::StackGuard g(L,20);
    lua_rawgeti (L,tblIdx,iKey);    // index( 1)
    lua_getfield(L,tblIdx,"bVal");  // index( 2)
    lua_getfield(L,tblIdx,"iVal");  // index( 3)
    lua_getfield(L,tblIdx,"dVal");  // index( 4)
    lua_getfield(L,tblIdx,"sVal");  // index( 5)
    lua_getfield(L,tblIdx,"mkVal"); // index( 6)
    lua_rawgeti (L,tblIdx,1);       // index( 7)
    lua_rawgeti (L,tblIdx,2);       // index( 8)
    lua_rawgeti (L,tblIdx,3);       // index( 9)
    lua_rawgeti (L,tblIdx,4);       // index(10)
    lua_rawgeti (L,tblIdx,5);       // index(11)
    aliLuaCore::StackGuard sub(L,0);
    lua_getfield(L,tblIdx, "sub1");
    lua_rawgeti (L,tblIdx, 6);
    lua_getfield(L,sub.Index(1),"sub1Key");
    lua_getfield(L,sub.Index(2),"sub2Key");
    //
    // verify table contents
    // items by key
    ASSERT_EQ(   i1Val,          lua_tointeger(L, g.Index( 1)));
    ASSERT_EQ(   bVal,           lua_toboolean(L, g.Index( 2)));
    ASSERT_EQ(   i2Val,          lua_tointeger(L, g.Index( 3)));
    ASSERT_NEAR( dVal,           lua_tonumber (L, g.Index( 4)), locals::dDiff);
    ASSERT_STREQ(s1Val.c_str(),  lua_tostring (L, g.Index( 5)));
    ASSERT_STREQ(s2Val.c_str(),  lua_tostring (L, g.Index( 6)));
    // items by index
    ASSERT_EQ(   bVal,           lua_toboolean(L, g.Index( 7)));
    ASSERT_EQ(   i2Val,          lua_tointeger(L, g.Index( 8)));
    ASSERT_NEAR( dVal,           lua_tonumber (L, g.Index( 9)), locals::dDiff);
    ASSERT_STREQ(s3Val.c_str(),  lua_tostring (L, g.Index(10)));
    ASSERT_STREQ(s2Val.c_str(),  lua_tostring (L, g.Index(11)));
    // sub tables
    ASSERT_STREQ(s4Val.c_str(),lua_tostring(L,sub.Index(3)));
    ASSERT_STREQ(s5Val.c_str(),lua_tostring(L,sub.Index(4)));
  };
  verify(g.Index(1));
  verify(g.Index(2));

  lua_close(L);
}
