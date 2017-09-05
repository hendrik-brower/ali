#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using BPtr      = aliLuaTest::Util::BPtr;
  using IPtr      = aliLuaTest::Util::IPtr;
  using IVec      = aliLuaTest::Util::IVec;
  using IVecPtr   = aliLuaTest::Util::IVecPtr;
  using Functions = aliLuaCore::Functions;
  using LuaFn     = aliLuaCore::LuaFn;
  using WrapperFn = aliLuaCore::WrapperFn;
  
}


TEST(aliLuaCoreFunctions, wrap1) {
  IVecPtr vec(new IVec);
  lua_State *L = nullptr;
  LuaFn luaFn = [=](lua_State *) {
    vec->push_back(2);
    return 10;
  };
  WrapperFn wrapperFn = [=](const LuaFn &orig, lua_State *) {
    vec->push_back(1);
    orig(L);
    vec->push_back(3);
    return 20;
  };
  LuaFn fn = Functions::Wrap(luaFn, wrapperFn);
  int rc = fn(L);
  ASSERT_EQ(rc, 20);
  ASSERT_EQ(vec->size(),3u);
  ASSERT_EQ(vec->operator[](0),1);
  ASSERT_EQ(vec->operator[](1),2);
  ASSERT_EQ(vec->operator[](2),3);
}

TEST(aliLuaCoreFunctions, wrap2) {
  lua_State *L = nullptr;
  IPtr       iPtr(new int(0));
  BPtr       bPtr(new bool(false));;
  LuaFn      fn = Functions::Wrap<int>(iPtr, [=](const IPtr &p, lua_State *) {
      *bPtr = p.get()==iPtr.get();
      return 30;
    });
  int rc = fn(L);
  ASSERT_EQ(rc, 30);
  ASSERT_TRUE(*bPtr);
}

TEST(aliLuaCoreFunctions, wrap3) {
  lua_State *L = nullptr;
  IPtr       iPtr(new int(0));
  BPtr       bPtr(new bool(false));;
  IVecPtr    vec(new IVec);
  LuaFn      luaFn = [=](lua_State *) {
    vec->push_back(2);
    return 3;
  };
  LuaFn      fn = Functions::Wrap<int>(iPtr,
				       luaFn,
				       [=](const IPtr &p, const LuaFn &orig, lua_State *) {
					 vec->push_back(1);
					 vec->push_back(orig(L));
					 vec->push_back(4);
					 *bPtr = p.get()==iPtr.get();
					 return 5;
				       });
  vec->push_back(fn(L));
  ASSERT_TRUE(*bPtr);
  ASSERT_EQ(vec->operator[](0),1);
  ASSERT_EQ(vec->operator[](1),2);
  ASSERT_EQ(vec->operator[](2),3);
  ASSERT_EQ(vec->operator[](3),4);
  ASSERT_EQ(vec->operator[](4),5);

}
