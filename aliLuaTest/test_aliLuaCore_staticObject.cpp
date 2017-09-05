#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>
#include <lua.hpp>

namespace {
  const std::string name = "myObj";
  char suffixChar = 'a'-1;

  struct MyObj1 {
    MyObj1(int iVal_, int jVal_) : iVal(iVal_), jVal(jVal_) {}
    virtual ~MyObj1() {}
    friend std::ostream &operator<<(std::ostream &out, MyObj1 &) {
      out << name;
      return out;
    }
    int IVal() const { return iVal; }
    int JVal() const { return jVal; }
  private:
    int iVal;
    int jVal;
  };
  struct MyObj2 : public MyObj1 {
    MyObj2(int iVal_, int jVal_) : MyObj1(iVal_, jVal_) {}
  };
  struct MyObj3 : public MyObj1 {
    MyObj3(int iVal_, int jVal_) : MyObj1(iVal_, jVal_) {}
  };

  using Exec         = aliLuaCore::Exec;
  using ExecEngine   = aliLuaExt::ExecEngine;
  using FnMap        = aliLuaCore::FunctionMap;
  using Future       = aliLuaCore::Future;
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


  FnMap::Ptr  fnMap1;
  FnMap::Ptr  fnMap2;
  FnMap::Ptr  fnMap3;
  using OBJ1        = aliLuaCore::StaticObject<MyObj1>;
  using OBJ2        = aliLuaCore::StaticObject<MyObj2>;
  using OBJ3        = aliLuaCore::StaticObject<MyObj3>;

  
  struct aliLuaCoreStaticObjectTest : testing::Test {
    static void SerializeObj  (lua_State *L,
			       int idx,
			       Serializer &sObj);
    static int  DeserializeObj(lua_State *L,
			       Deserializer &dObj);
    void SetUp() {
      pool   = Pool::Create(name, 1);
      exec   = ExecEngine::Create(name, pool);
    }
    void TearDown() {
      pool.reset();
      exec.reset();
    }
    static void SetUpTestCase() {
      fnMap1 = FnMap::Create(name + ".fnMap1");
      fnMap2 = FnMap::Create(name + ".fnMap2");
      fnMap3 = FnMap::Create(name + ".fnMap3");
      OBJ1::Init("staticObject-1", fnMap1, false);
      OBJ2::Init("staticObject-2", fnMap2, true);
      OBJ3::Init("staticObject-3", fnMap3, true, SerializeObj, DeserializeObj);
    }
    static void TearDownTestCase() {
      fnMap1.reset();
      fnMap2.reset();
      fnMap3.reset();
      OBJ1::Fini();
      OBJ2::Fini();
      OBJ3::Fini();
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
  };

  void aliLuaCoreStaticObjectTest::SerializeObj(lua_State *L,
						int index,
						Serializer &sObj) {
    OBJ3::TPtr p = OBJ3::Get(L,index,false);
    sObj.WriteInt(p->IVal());
    sObj.WriteInt(p->JVal());
  }
  int  aliLuaCoreStaticObjectTest::DeserializeObj(lua_State *L,
						  Deserializer &dObj) {
    OBJ3::TPtr p(new MyObj3(dObj.ReadInt(),
			    dObj.ReadInt()));
    return OBJ3::Make(L,p);
  }

}

TEST_F(aliLuaCoreStaticObjectTest, mtPtr) {
  ASSERT_NE(nullptr, OBJ1::GetMT());
  ASSERT_NE(nullptr, OBJ2::GetMT());
  ASSERT_NE(nullptr, OBJ3::GetMT());
}
TEST_F(aliLuaCoreStaticObjectTest, create) {
  ASSERT_FALSE(OBJ1::GetMT()->CanDup());
  ASSERT_TRUE( OBJ2::GetMT()->CanDup());
  ASSERT_TRUE( OBJ3::GetMT()->CanDup());
  ASSERT_FALSE(OBJ1::GetMT()->CanSerialize());
  ASSERT_FALSE(OBJ2::GetMT()->CanSerialize());
  ASSERT_TRUE( OBJ3::GetMT()->CanSerialize());
}

TEST_F(aliLuaCoreStaticObjectTest, ToAndGetString) {
  IPtr rc1(new int(0));
  IPtr rc2(new int(0));
  IPtr top(new int(0));
  BPtr b1(new bool(false));
  OBJ1::TPtr p(new MyObj1(4,5));
  std::string str = OBJ1::GetString(p);
  ASSERT_STREQ(str.c_str(),name.c_str());
  Future::Ptr fPtr = Future::Create();
  OBJ1::Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      *rc1 = OBJ1::Make(L, p);
      *rc2 = OBJ1::ToString(L);
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

TEST_F(aliLuaCoreStaticObjectTest, MakeMakeWeakIsAndGet) {
  IPtr        c1(new int(0));
  IPtr        c2(new int(0));
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  OBJ1::Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ1::TPtr  p(new MyObj1(4,5));
      MyObj1     *addr = p.get();
      *c1 = OBJ1::Make(L, p);
      *c2 = OBJ1::MakeWeak(L,p);
      *b1 = OBJ1::Is(L,1);
      *b2 = OBJ1::Is(L,2);
      OBJ1::TPtr p1 = OBJ1::Get(L,1,true);
      OBJ1::TPtr p2 = OBJ1::Get(L,2,true);
      *b3 = p1.get()==addr;
      *b4 = p2.get()==addr;
      p.reset();
      p1.reset();
      p2.reset();
      p1 = OBJ1::Get(L,1,true);
      *b5 = p1.get()==addr;
      p1.reset();
      lua_remove(L,1); // remove strong, weak now at index 1
      lua_gc(L, LUA_GCCOLLECT, 0);
      p2 = OBJ1::Get(L,1,true); 
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

TEST_F(aliLuaCoreStaticObjectTest, GetMakeAndGetMakeWeak) {
  IPtr        c1(new int(0));
  IPtr        c2(new int(0));
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  OBJ1::Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ1::TPtr  p(new MyObj1(4,5));
      MyObj1     *addr     = p.get();
      MakeFn      make     = OBJ1::GetMakeFn(p);
      MakeFn      makeWeak = OBJ1::GetMakeWeakFn(p);
      *c1 = make(L);
      *c2 = makeWeak(L);
      make = Values::MakeNothing; // release the pointer wrapped in the make
      *b1 = OBJ1::Is(L,1);
      *b2 = OBJ1::Is(L,2);
      OBJ1::TPtr p1 = OBJ1::Get(L,1,true);
      OBJ1::TPtr p2 = OBJ1::Get(L,2,true);
      *b3 = p1.get()==addr;
      *b4 = p2.get()==addr;
      p.reset();
      p1.reset();
      p2.reset();
      p1 = OBJ1::Get(L,1,true);
      *b5 = p1.get()==addr;
      p1.reset();
      lua_remove(L,1); // remove strong, weak now at index 1
      lua_gc(L, LUA_GCCOLLECT, 0);
      p2 = OBJ1::Get(L,1,true); 
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
TEST_F(aliLuaCoreStaticObjectTest, TableGet) {
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  BPtr        b7(new bool(false));
  BPtr        b8(new bool(false));
  Future::Ptr fPtr = Future::Create();
  OBJ1::Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ1::TPtr  p1(new MyObj1(4,5));
      OBJ1::TPtr  p2;
      MyObj1     *addr         = p1.get();
      const int   tblIndex     = 1;
      const char *valField     = "valField";
      const char *nullValField = "nullField";
      const char *nilField     = "nilField";
      const int   valIndex     = 20;
      const int   nullValIndex = 21;
      const int   nilIndex     = 22;
      lua_newtable(L); // tblIndex
      OBJ1::Make(L,p1); lua_setfield(L,tblIndex, valField);
      OBJ1::Make(L,p1); lua_seti    (L,tblIndex, valIndex);
      OBJ1::Make(L,p2); lua_setfield(L,tblIndex, nullValField); // null pointer
      OBJ1::Make(L,p2); lua_seti    (L,tblIndex, nullValIndex); // null pointer
      OBJ1::TPtr fieldVal;
      OBJ1::TPtr indexVal;
      OBJ1::GetTableValue(L,tblIndex, valField, fieldVal, true);
      OBJ1::GetTableIndex(L,tblIndex, valIndex, indexVal, true);
      *b1 = fieldVal.get()==addr;
      *b2 = indexVal.get()==addr;
      OBJ1::GetTableValue(L,tblIndex, nullValField, fieldVal, true);
      OBJ1::GetTableIndex(L,tblIndex, nullValIndex, indexVal, true);
      *b3 = fieldVal.get()==nullptr;
      *b4 = indexVal.get()==nullptr;
      OBJ1::GetTableValue(L,tblIndex, nilField, fieldVal, true);
      OBJ1::GetTableIndex(L,tblIndex, nilIndex, indexVal, true);
      *b5 = fieldVal.get()==nullptr;
      *b6 = indexVal.get()==nullptr;
      *b7 = TestUtil::DidThrow([=]() {
	  OBJ1::TPtr val;
	  OBJ1::GetTableValue(L,tblIndex, nullValField, val, false);
	});
      *b8 = TestUtil::DidThrow([=]() {
	  OBJ1::TPtr val;
	  OBJ1::GetTableIndex(L,tblIndex, nullValIndex, val, false);
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
TEST_F(aliLuaCoreStaticObjectTest, TableValueIs) {
  BPtr        b1(new bool(false));
  BPtr        b2(new bool(false));
  BPtr        b3(new bool(false));
  BPtr        b4(new bool(false));
  BPtr        b5(new bool(false));
  BPtr        b6(new bool(false));
  Future::Ptr fPtr = Future::Create();
  OBJ1::Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      OBJ1::TPtr  p1(new MyObj1(4,5));
      OBJ1::TPtr  p2;
      const int   tblIndex     = 1;
      const char *valField     = "valField";
      const char *nullValField = "nullField";
      const char *nilField     = "nilField";
      const int   valIndex     = 20;
      const int   nullValIndex = 21;
      const int   nilIndex     = 22;
      lua_newtable(L); // tblIndex
      OBJ1::Make(L,p1); lua_setfield(L,tblIndex, valField);
      OBJ1::Make(L,p1); lua_seti    (L,tblIndex, valIndex);
      OBJ1::Make(L,p2); lua_setfield(L,tblIndex, nullValField); // null pointer
      OBJ1::Make(L,p2); lua_seti    (L,tblIndex, nullValIndex); // null pointer
      // Table value is
      *b1 = OBJ1::TableValueIs(L, tblIndex, valField);
      *b2 = OBJ1::TableValueIs(L, tblIndex, nullValField);
      *b3 = OBJ1::TableValueIs(L, tblIndex, nilField)==false;
      // Table index Is
      *b4 = OBJ1::TableIndexIs(L, tblIndex, valIndex);
      *b5 = OBJ1::TableIndexIs(L, tblIndex, nullValIndex);
      *b6 = OBJ1::TableIndexIs(L, tblIndex, nilIndex)==false;
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
