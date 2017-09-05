#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <functional>
#include <lua.hpp>

namespace {
  using MakeFn     = aliLuaCore::MakeFn;
  using MakeFnVec  = aliLuaCore::MakeFnVec;
  using StackGuard = aliLuaCore::StackGuard;
  using Values     = aliLuaCore::Values;
}

struct aliLuaCoreValues : testing::Test {
  const double dDiff = 0.00000001;
  lua_State *L;
  void SetUp() {
    L = luaL_newstate();
  }
  void TearDown() {
    lua_close(L);
  }
};

TEST_F(aliLuaCoreValues, GetString) {
  StackGuard g(L,10);
  lua_pushstring(L, "abc");           // index 1
  lua_pushnil(L);                     // index 2
  lua_pushnumber(L,25.4);             // index 3
  lua_pushinteger(L,1);               // index 4
  lua_pushcfunction(L,&lua_gettop);   // index 5
  std::string i1 = Values::GetString(L,g.Index(1)); // abc
  std::string i2 = Values::GetString(L,g.Index(2)); // nil
  std::string i3 = Values::GetString(L,g.Index(3)); // number
  std::string i4 = Values::GetString(L,g.Index(4)); // integer
  std::string i5 = Values::GetString(L,g.Index(5)); // function
  std::string i6 = Values::GetString(L,g.Index(6)); // none
  ASSERT_STREQ(i1.c_str(), "abc");
  ASSERT_STREQ(i2.c_str(), "");
  ASSERT_STREQ(i3.c_str(), "25.4");
  ASSERT_STREQ(i4.c_str(), "1");
  ASSERT_STREQ(i5.c_str(), "");
  ASSERT_STREQ(i6.c_str(), "");
}

TEST_F(aliLuaCoreValues, MakeValues) {
  StackGuard         g(L,0); /// use 0 to make sure make fn's call lua_check
  const int          iVal = 2341;
  const double       dVal = 52.348;
  const std::string  sVal = "some string value";
  ASSERT_EQ(0, Values::MakeNothing(L));
  ASSERT_EQ(g.Diff(),0) << "make nothing pushed a value";
  ASSERT_EQ(1, Values::MakeNil(L));
  ASSERT_EQ(g.Diff(),1) << "make nil did not push a single value";  
  ASSERT_EQ(1, Values::MakeTrue(L));
  ASSERT_EQ(g.Diff(),2) << "make true did not push a single value";
  ASSERT_EQ(1, Values::MakeFalse(L));
  ASSERT_EQ(g.Diff(),3) << "make false did not push a single value";
  ASSERT_EQ(1, Values::MakeInteger(L,iVal));
  ASSERT_EQ(g.Diff(),4) << "make integer did not push a single value";
  ASSERT_EQ(1, Values::MakeDouble(L, dVal));
  ASSERT_EQ(g.Diff(),5) << "make double did not push a single value";
  ASSERT_EQ(1, Values::MakeString(L, sVal));
  ASSERT_EQ(g.Diff(),6) << "make string did not push a single value";
  ASSERT_EQ(1, Values::MakeEmptyString(L));
  ASSERT_EQ(g.Diff(),7) << "make empty string did not push a single value";
  
  ASSERT_TRUE(lua_isnil    (L,1));
  ASSERT_TRUE(lua_isboolean(L,2));
  ASSERT_TRUE(lua_isboolean(L,3));
  ASSERT_TRUE(lua_isinteger(L,4));
  ASSERT_TRUE(lua_isnumber (L,5));
  ASSERT_TRUE(lua_isstring (L,6));
  ASSERT_TRUE(lua_isstring (L,7));

  ASSERT_TRUE( lua_toboolean(L,2));
  ASSERT_FALSE(lua_toboolean(L,3));
  ASSERT_EQ(   lua_tointeger(L,4), iVal);
  ASSERT_NEAR( lua_tonumber (L,5), dVal, dDiff);
  ASSERT_STREQ(lua_tostring (L,6), sVal.c_str());
  ASSERT_STREQ(lua_tostring (L,7), "");
}

TEST_F(aliLuaCoreValues, GetMakeValues) {
  StackGuard g(L,0); /// use 0 to try to catch fn's that don't call lua_check
  const bool         bVal = true;
  const int          iVal = 9354;
  const double       dVal = 373.8907;
  const std::string  sVal = "some string of this or that";
  MakeFn mkNothing  = Values::GetMakeNothingFn();
  MakeFn mkNil      = Values::GetMakeNilFn();
  MakeFn mkBool     = Values::GetMakeBoolFn(bVal);
  MakeFn mkTrue     = Values::GetMakeTrueFn();
  MakeFn mkFalse    = Values::GetMakeFalseFn();
  MakeFn mkInt      = Values::GetMakeIntegerFn(iVal);
  MakeFn mkDbl      = Values::GetMakeDoubleFn(dVal);
  MakeFn mkStr      = Values::GetMakeStringFn(sVal);
  MakeFn mkEmptyStr = Values::GetMakeEmptyStringFn();
  MakeFnVec mkVec;
  mkVec.push_back(mkNothing);
  mkVec.push_back(mkNil);
  mkVec.push_back(mkBool);
  mkVec.push_back(mkTrue);
  mkVec.push_back(mkFalse);
  mkVec.push_back(mkInt);
  mkVec.push_back(mkDbl);
  mkVec.push_back(mkStr);
  mkVec.push_back(mkEmptyStr);
  MakeFn mkArray    = Values::GetMakeArrayFn(mkVec);
  mkVec.clear();
  ASSERT_EQ(0, mkNothing(L));
  ASSERT_EQ(1, mkNil(L));
  ASSERT_EQ(1, mkBool(L));
  ASSERT_EQ(1, mkTrue(L));
  ASSERT_EQ(1, mkFalse(L));
  ASSERT_EQ(1, mkInt(L));
  ASSERT_EQ(1, mkDbl(L));
  ASSERT_EQ(1, mkStr(L));
  ASSERT_EQ(1, mkEmptyStr(L));
  ASSERT_EQ(8, mkArray(L));

  std::function<void(int)> check = [&](int base) {
    ASSERT_TRUE(lua_isnil    (L,base+1));
    ASSERT_TRUE(lua_isboolean(L,base+2));
    ASSERT_TRUE(lua_isboolean(L,base+3));
    ASSERT_TRUE(lua_isboolean(L,base+4));
    ASSERT_TRUE(lua_isinteger(L,base+5));
    ASSERT_TRUE(lua_isnumber (L,base+6));
    ASSERT_TRUE(lua_isstring (L,base+7));
    ASSERT_TRUE(lua_isstring (L,base+8));
    
    ASSERT_EQ  ( lua_toboolean(L,base+2), bVal);
    ASSERT_TRUE( lua_toboolean(L,base+3));
    ASSERT_FALSE(lua_toboolean(L,base+4));
    ASSERT_EQ(   lua_tointeger(L,base+5), iVal);
    ASSERT_NEAR( lua_tonumber (L,base+6), dVal, dDiff);
    ASSERT_STREQ(lua_tostring (L,base+7), sVal.c_str());
    ASSERT_STREQ(lua_tostring (L,base+8), "");
  };
  check(0);
  check(8);
}

TEST_F(aliLuaCoreValues, GetMakeFn) {
  StackGuard g(L,4); // just enough to cover the manual pushes
  const int    iVal = 9246;
  const double dVal = 224.9324;
  const bool   bVal = true;

  lua_pushinteger(L,iVal); // index 1
  lua_pushnumber(L,dVal);  // index 2
  lua_pushboolean(L,bVal); // index 3
  lua_pushnil(L);          // index 4
  ASSERT_EQ(g.Diff(),4);
  MakeFn all    = Values::GetMakeFnForAll(L);
  MakeFn idx2   = Values::GetMakeFnForIndex(L,2);
  MakeFn idx3   = Values::GetMakeFnForIndex(L,3);
  MakeFn byCnt  = Values::GetMakeFnByCount(L,2,2);
  MakeFn idx234 = Values::GetMakeFnRemaining(L,2);

  StackGuard g2(L,0);
  ASSERT_EQ(4, all(   L)); ASSERT_EQ(g2.Diff(),  4);
  ASSERT_EQ(1, idx2(  L)); ASSERT_EQ(g2.Diff(),  5);
  ASSERT_EQ(1, idx3(  L)); ASSERT_EQ(g2.Diff(),  6);
  ASSERT_EQ(2, byCnt( L)); ASSERT_EQ(g2.Diff(),  8);
  ASSERT_EQ(3, idx234(L)); ASSERT_EQ(g2.Diff(), 11);

  ASSERT_EQ(  iVal, lua_tointeger(L, g2.Index(1))       ); // all    - iVal
  ASSERT_NEAR(dVal, lua_tonumber (L, g2.Index(2)), dDiff); // all    - dVal
  ASSERT_EQ(  bVal, lua_toboolean(L, g2.Index(3))       ); // all    - bVal
  ASSERT_TRUE(      lua_isnil    (L, g2.Index(4))       ); // all    - nil
  ASSERT_NEAR(dVal, lua_tonumber (L, g2.Index(5)), dDiff); // idx2   - dVal
  ASSERT_EQ(  bVal, lua_toboolean(L, g2.Index(6))       ); // idx3   - bVal
  ASSERT_NEAR(dVal, lua_tonumber (L, g2.Index(7)), dDiff); // byCnt  - dVal
  ASSERT_EQ(  bVal, lua_toboolean(L, g2.Index(8))       ); // byCnt  - bVal
  ASSERT_NEAR(dVal, lua_tonumber (L, g2.Index(9)), dDiff); // idx234 - dVal
  ASSERT_EQ(  bVal, lua_toboolean(L, g2.Index(10))      ); // idx234 - bVal
  ASSERT_TRUE(      lua_isnil    (L, g2.Index(11))      ); // idx234 - nil
}


TEST_F(aliLuaCoreValues, ConstructSequence) {
  StackGuard g(L,5);
  MakeFn seq = Values::ConstructAsSequence([](lua_State *L) {
      lua_pushinteger(L,1);
      lua_pushinteger(L,3);
      lua_pushinteger(L,5);
      lua_pushinteger(L,7);
      return 4;
    });
  seq(L);
  ASSERT_EQ(1, g.Diff());
  ASSERT_TRUE(lua_istable(L,g.Index(1)));
  lua_rawgeti(L, g.Index(1),1);
  lua_rawgeti(L, g.Index(1),2);
  lua_rawgeti(L, g.Index(1),3);
  lua_rawgeti(L, g.Index(1),4);
  ASSERT_EQ(1, lua_tointeger(L,g.Index(2)));
  ASSERT_EQ(3, lua_tointeger(L,g.Index(3)));
  ASSERT_EQ(5, lua_tointeger(L,g.Index(4)));
  ASSERT_EQ(7, lua_tointeger(L,g.Index(5)));
}
