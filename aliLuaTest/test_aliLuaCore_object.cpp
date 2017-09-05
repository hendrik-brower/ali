#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>
#include <lua.hpp>

namespace {
  const std::string name = "myObj";
  char suffixChar = 'a'-1;

  struct MyObj {
    MyObj(int iVal_, int jVal_) : iVal(iVal_), jVal(jVal_) {}
    friend std::ostream &operator<<(std::ostream &out, MyObj &) {
      out << name;
      return out;
    }
    int IVal() const { return iVal; }
    int JVal() const { return jVal; }
  private:
    int iVal;
    int jVal;
  };

  using Exec         = aliLuaCore::Exec;
  using ExecEngine   = aliLuaExt::ExecEngine;
  using FnMap        = aliLuaCore::FunctionMap;
  using Future       = aliLuaCore::Future;
  using OBJ          = aliLuaCore::Object<MyObj>;
  using MakeFn       = aliLuaCore::MakeFn;
  using Util         = aliLuaCore::Util;
  using Values       = aliLuaCore::Values;
  using BPtr         = aliLuaTest::Util::BPtr;
  using IPtr         = aliLuaTest::Util::IPtr;
  using TestUtil     = aliLuaTest::Util;
  using Serialize    = aliSystem::Codec::Serialize;
  using Serializer   = aliSystem::Codec::Serializer;
  using Deserialize  = aliSystem::Codec::Deserialize;
  using Deserializer = aliSystem::Codec::Deserializer;
  using Pool         = aliSystem::Threading::Pool;
  struct aliLuaCoreObjectTest : testing::Test {
    static void SerializeObj  (lua_State  *L,
			       int         idx,
			       Serializer &sPtr);
    static int  DeserializeObj(lua_State    *L,
			       Deserializer &dPtr);
    void SetUp() {
      pool   = Pool::Create(name, 1);
      exec   = ExecEngine::Create(name, pool);
      fnMap1 = FnMap::Create(name + ".fnMap1");
      fnMap2 = FnMap::Create(name + ".fnMap2");
      fnMap3 = FnMap::Create(name + ".fnMap3");
      oPtr1  = OBJ::Create(name+Suffix(1), fnMap1, false);
      oPtr2  = OBJ::Create(name+Suffix(2), fnMap2, true);
      oPtr3  = OBJ::Create(name+Suffix(3),
			   fnMap3,
			   true,
			   SerializeObj,
			   DeserializeObj);
    }
    void TearDown() {
      pool.reset();
      exec.reset();
      fnMap1.reset();
      fnMap2.reset();
      fnMap3.reset();
      oPtr1.reset();
      oPtr2.reset();
      oPtr3.reset();
    }
    std::string Suffix(int num) {
      const static char *numVal = "01234567890";
      std::string rtn = ".";
      if (num==1) { ++suffixChar; }
      rtn.push_back(suffixChar);
      rtn.push_back(numVal[num]);
      return rtn;
    }
    Pool::Ptr   pool;
    Exec::Ptr   exec;
    FnMap::Ptr  fnMap1;
    FnMap::Ptr  fnMap2;
    FnMap::Ptr  fnMap3;
    OBJ::Ptr    oPtr1;
    OBJ::Ptr    oPtr2;
    OBJ::Ptr    oPtr3;
  };

  void aliLuaCoreObjectTest::SerializeObj(lua_State *,
					  int ,
					  Serializer &sObj) {
    // this is not valid, just a dummy implementation for testing
    sObj.WriteInt(5); // some junk for iVal
    sObj.WriteInt(9); // some junk for jval
  }
  int  aliLuaCoreObjectTest::DeserializeObj(lua_State *,
					    Deserializer &dObj) {
    // this is not valid, just a dummy implementation for testing
    OBJ::TPtr p(new MyObj(dObj.ReadInt(),
			  dObj.ReadInt()));
    // don't have the oPtr3->Make(L,p);
    return 0;
  }

}

TEST_F(aliLuaCoreObjectTest, mtPtr) {
  ASSERT_NE(nullptr, oPtr1->GetMT());
  ASSERT_NE(nullptr, oPtr2->GetMT());
  ASSERT_NE(nullptr, oPtr3->GetMT());
  ASSERT_NE(nullptr, oPtr1->GetMT().get());
  ASSERT_NE(nullptr, oPtr2->GetMT().get());
  ASSERT_NE(nullptr, oPtr3->GetMT().get());
}
TEST_F(aliLuaCoreObjectTest, create) {
  ASSERT_FALSE(oPtr1->GetMT()->CanDup());
  ASSERT_TRUE( oPtr2->GetMT()->CanDup());
  ASSERT_TRUE( oPtr3->GetMT()->CanDup());
  ASSERT_FALSE(oPtr1->GetMT()->CanSerialize());
  ASSERT_FALSE(oPtr2->GetMT()->CanSerialize());
  ASSERT_TRUE( oPtr3->GetMT()->CanSerialize());
}

TEST_F(aliLuaCoreObjectTest, ToAndGetString) {
  IPtr rc1(new int(0));
  IPtr rc2(new int(0));
  IPtr top(new int(0));
  BPtr b1(new bool(false));
  OBJ::TPtr p(new MyObj(4,5));
  std::string str = oPtr1->GetString(p);
  ASSERT_STREQ(str.c_str(),name.c_str());
  Future::Ptr fPtr = Future::Create();
  oPtr1->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      *rc1 = oPtr1->Make(L, p);
      *rc2 = oPtr1->ToString(L);
      *top = lua_gettop(L);
      const char *str = lua_tostring(L,-1);
      *b1 = name==str;
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*rc1,1);
  ASSERT_EQ(*rc2,1);
  ASSERT_EQ(*top,2);
  ASSERT_TRUE(*b1);
}

TEST_F(aliLuaCoreObjectTest, MakeMakeWeakIsAndGet) {
  IPtr        c1(new int(0));
  IPtr        c2(new int(0));
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  oPtr1->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ::TPtr  p(new MyObj(4,5));
      MyObj     *addr = p.get();
      *c1 = oPtr1->Make(L, p);
      *c2 = oPtr1->MakeWeak(L,p);
      *b1 = oPtr1->Is(L,1);
      *b2 = oPtr1->Is(L,2);
      OBJ::TPtr p1 = oPtr1->Get(L,1,true);
      OBJ::TPtr p2 = oPtr1->Get(L,2,true);
      *b3 = p1.get()==addr;
      *b4 = p2.get()==addr;
      p.reset();
      p1.reset();
      p2.reset();
      p1 = oPtr1->Get(L,1,true);
      *b5 = p1.get()==addr;
      p1.reset();
      lua_remove(L,1); // remove strong, weak now at index 1
      lua_gc(L, LUA_GCCOLLECT, 0);
      p2 = oPtr1->Get(L,1,true); 
      *b6 = p2.get()==nullptr;
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*c1,1) << "make returns 1";
  ASSERT_EQ(*c2,1) << "make returns 1";
  ASSERT_TRUE(*b1) << "object created";
  ASSERT_TRUE(*b2) << "weak object created";
  ASSERT_TRUE(*b3) << "got ptr";
  ASSERT_TRUE(*b4) << "got weak ptr";
  ASSERT_TRUE(*b5) << "ptr was held";
  ASSERT_TRUE(*b6) << "weak ptr was released";
}

TEST_F(aliLuaCoreObjectTest, GetMakeAndGetMakeWeak) {
  IPtr        c1(new int(0));
  IPtr        c2(new int(0));
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  oPtr1->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ::TPtr  p(new MyObj(4,5));
      MyObj     *addr     = p.get();
      MakeFn     make     = oPtr1->GetMakeFn(p);
      MakeFn     makeWeak = oPtr1->GetMakeWeakFn(p);
      *c1 = make(L);
      *c2 = makeWeak(L);
      make = Values::MakeNothing; // release the pointer wrapped in the make
      *b1 = oPtr1->Is(L,1);
      *b2 = oPtr1->Is(L,2);
      OBJ::TPtr p1 = oPtr1->Get(L,1,true);
      OBJ::TPtr p2 = oPtr1->Get(L,2,true);
      *b3 = p1.get()==addr;
      *b4 = p2.get()==addr;
      p.reset();
      p1.reset();
      p2.reset();
      p1 = oPtr1->Get(L,1,true);
      *b5 = p1.get()==addr;
      p1.reset();
      lua_remove(L,1); // remove strong, weak now at index 1
      lua_gc(L, LUA_GCCOLLECT, 0);
      p2 = oPtr1->Get(L,1,true); 
      *b6 = p2.get()==nullptr;
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*c1,1) << "make returns 1";
  ASSERT_EQ(*c2,1) << "make returns 1";
  ASSERT_TRUE(*b1) << "object created";
  ASSERT_TRUE(*b2) << "weak object created";
  ASSERT_TRUE(*b3) << "got ptr";
  ASSERT_TRUE(*b4) << "got weak ptr";
  ASSERT_TRUE(*b5) << "ptr was held";
  ASSERT_TRUE(*b6) << "weak ptr was released";
}
TEST_F(aliLuaCoreObjectTest, TableGet) {
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  BPtr        b7(new bool(false));
  BPtr        b8(new bool(false));
  Future::Ptr fPtr = Future::Create();
  oPtr1->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ::TPtr   p1(new MyObj(4,5));
      OBJ::TPtr   p2;
      MyObj      *addr         = p1.get();
      const int   tblIndex     = 1;
      const char *valField     = "valField";
      const char *nullValField = "nullField";
      const char *nilField     = "nilField";
      const int   valIndex     = 20;
      const int   nullValIndex = 21;
      const int   nilIndex     = 22;
      lua_newtable(L); // tblIndex
      oPtr1->Make(L,p1); lua_setfield(L,tblIndex, valField);
      oPtr1->Make(L,p1); lua_seti    (L,tblIndex, valIndex);
      oPtr1->Make(L,p2); lua_setfield(L,tblIndex, nullValField); // null pointer
      oPtr1->Make(L,p2); lua_seti    (L,tblIndex, nullValIndex); // null pointer
      OBJ::TPtr fieldVal;
      OBJ::TPtr indexVal;
      oPtr1->GetTableValue(L,tblIndex, valField, fieldVal, true);
      oPtr1->GetTableIndex(L,tblIndex, valIndex, indexVal, true);
      *b1 = fieldVal.get()==addr;
      *b2 = indexVal.get()==addr;
      oPtr1->GetTableValue(L,tblIndex, nullValField, fieldVal, true);
      oPtr1->GetTableIndex(L,tblIndex, nullValIndex, indexVal, true);
      *b3 = fieldVal.get()==nullptr;
      *b4 = indexVal.get()==nullptr;
      oPtr1->GetTableValue(L,tblIndex, nilField, fieldVal, true);
      oPtr1->GetTableIndex(L,tblIndex, nilIndex, indexVal, true);
      *b5 = fieldVal.get()==nullptr;
      *b6 = indexVal.get()==nullptr;
      *b7 = TestUtil::DidThrow([=]() {
	  OBJ::TPtr val;
	  oPtr1->GetTableValue(L,tblIndex, nullValField, val, false);
	});
      *b8 = TestUtil::DidThrow([=]() {
	  OBJ::TPtr val;
	  oPtr1->GetTableIndex(L,tblIndex, nullValIndex, val, false);
	});
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*b1) << "value extracted from a field";
  ASSERT_TRUE(*b2) << "value extracted from an index";
  ASSERT_TRUE(*b3) << "null value extracted from a field";
  ASSERT_TRUE(*b4) << "null value extracted from an index";
  ASSERT_TRUE(*b5) << "nil field w/allwoNull=true get null";
  ASSERT_TRUE(*b6) << "nil index w/allowNull=true get null";
  ASSERT_TRUE(*b7) << "nil field w/allowNull=false throws";
  ASSERT_TRUE(*b8) << "nil index w/allowNull=false throws";
}
TEST_F(aliLuaCoreObjectTest, TableValueIs) {
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  oPtr1->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ::TPtr   p1(new MyObj(4,5));
      OBJ::TPtr   p2;
      const int   tblIndex     = 1;
      const char *valField     = "valField";
      const char *nullValField = "nullField";
      const char *nilField     = "nilField";
      const int   valIndex     = 20;
      const int   nullValIndex = 21;
      const int   nilIndex     = 22;
      lua_newtable(L); // tblIndex
      oPtr1->Make(L,p1); lua_setfield(L,tblIndex, valField);
      oPtr1->Make(L,p1); lua_seti    (L,tblIndex, valIndex);
      oPtr1->Make(L,p2); lua_setfield(L,tblIndex, nullValField); // null pointer
      oPtr1->Make(L,p2); lua_seti    (L,tblIndex, nullValIndex); // null pointer
      // Table value is
      *b1 = oPtr1->TableValueIs(L, tblIndex, valField);
      *b2 = oPtr1->TableValueIs(L, tblIndex, nullValField);
      *b3 = oPtr1->TableValueIs(L, tblIndex, nilField)==false;
      // Table index Is
      *b4 = oPtr1->TableIndexIs(L, tblIndex, valIndex);
      *b5 = oPtr1->TableIndexIs(L, tblIndex, nullValIndex);
      *b6 = oPtr1->TableIndexIs(L, tblIndex, nilIndex)==false;
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*b1) << "Is recognizes field value";
  ASSERT_TRUE(*b2) << "Is recognizes null field value";
  ASSERT_TRUE(*b3) << "Is does not recognize nil field";
  ASSERT_TRUE(*b4) << "Is recognizes index value";
  ASSERT_TRUE(*b5) << "Is recognizes null index value";
  ASSERT_TRUE(*b6) << "Is does not recognize nil index";
}
