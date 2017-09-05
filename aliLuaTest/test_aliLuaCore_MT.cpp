#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_exec.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

#define MT_NAME  "mt test obj"
#define TO_STRING_TEXT "mt test obj to string output"

namespace {
  using Exec         = aliLuaCore::Exec;
  using ExecEngine   = aliLuaExt::ExecEngine;
  using FMap         = aliLuaCore::FunctionMap;
  using Future       = aliLuaCore::Future;
  using MT           = aliLuaCore::MT;
  using MakeFn       = aliLuaCore::MakeFn;
  using Module       = aliLuaCore::Module;
  using StackGuard   = aliLuaCore::StackGuard;
  using Util         = aliLuaCore::Util;
  using Values       = aliLuaCore::Values;
  using BPtr         = aliLuaTest::Util::BPtr;
  using DPtr         = aliLuaTest::Util::DPtr;
  using IPtr         = aliLuaTest::Util::IPtr;
  using TestExec     = aliLuaTest::Exec;
  using TestUtil     = aliLuaTest::Util;
  using Serialize    = aliSystem::Codec::Serialize;
  using Serializer   = aliSystem::Codec::Serializer;
  using Deserialize  = aliSystem::Codec::Deserialize;
  using Deserializer = aliSystem::Codec::Deserializer;
  using Pool         = aliSystem::Threading::Pool;

  FMap::Ptr mtMap1;
  FMap::Ptr mtMap2;
  FMap::Ptr mtMap3;
  FMap::Ptr mtMap4;
  MT::Ptr   aNoDup;
  MT::Ptr   aDup;
  MT::Ptr   aSer;
  MT::Ptr   aDerived;

  struct Obj {
    using Ptr = std::shared_ptr<Obj>;
    using OnDestroy = std::function<void()>;
    Obj() {
      ++count;
    }
    Obj(const OnDestroy onDestroy_)
      : onDestroy(onDestroy_) {
      ++count;
    }
    virtual ~Obj() {
      --count;
      if (onDestroy) {
	onDestroy();
      }
    }
    static size_t Count() { return count; }
    int    iVal;
    double dVal;
  private:
    OnDestroy onDestroy;
    static size_t count;
  };
  size_t Obj::count = 0;
  struct ObjDerived : public Obj {
    using Ptr = std::shared_ptr<ObjDerived>;
    ObjDerived(const OnDestroy onDestroyBase, const OnDestroy onDestroy_)
      : Obj(onDestroyBase),
	onDestroy(onDestroy_) {
    }
    ~ObjDerived() {
      if (onDestroy) {
	onDestroy();
      }
    }
  private:
    OnDestroy onDestroy;       
  };

  using OBJ = aliLuaCore::StaticObject<Obj>;

  void Obj3ToString(lua_State  *L,
		    int         idx,
		    Serializer &sPtr);
  int  Obj3FromString(lua_State    *L,
		      Deserializer &dObj);

  void Init() {
    mtMap1 = FMap::Create("obj1 MT");
    mtMap1->Add("MakeTrue", Values::MakeTrue);
    mtMap2 = FMap::Create("obj2 MT");
    mtMap3 = FMap::Create("obj3 MT");
    mtMap4 = FMap::Create("obj4 MT");
    aNoDup = MT::Create("mtTestObject1",
			mtMap1,
			false);
    aDup = MT::Create("mtTestObject2",
		      mtMap2,
		      true);
    aSer = MT::Create("mtTestObject3",
		      mtMap3,
		      true,
		      Obj3ToString,
		      Obj3FromString);
    aDerived = MT::Create("mtTestObject4",
			  mtMap4,
			  true);
    aDup->AddDerived(aDerived);
    //
    // for script API testing
    FMap::Ptr fnMap = FMap::Create("global functions for mt testing");
    fnMap->Add("Create", [=](lua_State *L) {
	Obj::Ptr ptr(new Obj);
	ptr->iVal = lua_tointeger(L,1);
	ptr->dVal = lua_tonumber(L,2);
	return OBJ::Make(L, ptr);
      });
    FMap::Ptr mtMap = FMap::Create("obj functions");

    mtMap->Add("__tostring", [](lua_State *L) {
	lua_pushstring(L, TO_STRING_TEXT);
	return 1;
      });
    mtMap->Add("IVal", [=](lua_State *L) {
	OBJ::TPtr oPtr = OBJ::Get(L,1,false);
	lua_pushinteger(L,oPtr->iVal);
	return 1;
      });
    mtMap->Add("DVal", [=](lua_State *L) {
	OBJ::TPtr oPtr = OBJ::Get(L,1,false);
	lua_pushnumber(L,oPtr->dVal);
	return 1;
      });
    OBJ::Init(MT_NAME, mtMap, true);
    Module::Register("load mt test functions",
		     [=](const aliLuaCore::Exec::Ptr &ePtr) {
		       Util::LoadFnMap(ePtr, "lib.mtTest", fnMap);
		       OBJ::GetMT()->Register(ePtr);
		     });
  }
  void Fini() {
    aNoDup.reset();
    aDup.reset();
    aSer.reset();
    mtMap1.reset();
    mtMap2.reset();
    mtMap3.reset();
    OBJ::Fini();
  }
  
  void Obj3ToString(lua_State  *,
		    int         ,
		    Serializer &sObj) {
    sObj.WriteString("objX");
  }
  int Obj3FromString(lua_State    *L,
		     Deserializer &dObj) {
    std::string str;
    dObj.ReadString(str);
    THROW_IF(str!="objX", "Unexpectd input");
    Obj::Ptr oPtr(new Obj);
    return aSer->MakeObject(L,oPtr);
  }
  
  
  struct aliLuaCoreMT : testing::Test {
    Exec::Ptr exec;
    Pool::Ptr pool;
    void SetUp();
    void TearDown();
  };


  
  void aliLuaCoreMT::SetUp() {
    pool     = Pool::Create("test pool",2);
    exec     = ExecEngine::Create("test exec", pool);
  }
  void aliLuaCoreMT::TearDown() {
    exec.reset();
    pool.reset();
  }

  
}

void RegisterInitFini_MTTest(aliSystem::ComponentRegistry &cr) {
  aliSystem::Component::Ptr ptr = cr.Register("test_aliLuaCoreMT", Init, Fini);
  ptr->AddDependency("aliLuaCore");
}


TEST_F(aliLuaCoreMT, verifyFnFreeze) {
  // Verify the function map is frozen.
  // Though there is no issue in c++ code
  // that would require this, once one starts
  // pushing functions into Lua, Adding functions
  // becomes more complicated since the existing
  // interpreter's metatable would need to be updated.
  // To promote simplicity, the functionmaps are frozen
  // when the MT object is created.
  ASSERT_TRUE(mtMap1->IsFrozen());
  ASSERT_TRUE(mtMap2->IsFrozen());
  ASSERT_TRUE(mtMap3->IsFrozen());
}
TEST_F(aliLuaCoreMT, verifyReserved) {
  ASSERT_TRUE(mtMap1->IsDefined("__gc"));
  ASSERT_TRUE(mtMap1->IsDefined("__tostring"));
  ASSERT_TRUE(mtMap1->IsDefined("ToWeak"));
  ASSERT_TRUE(mtMap1->IsDefined("ToStrong"));
  ASSERT_TRUE(mtMap1->IsDefined("IsValid"));
  ASSERT_TRUE(mtMap1->IsDefined("Release"));
  ASSERT_TRUE(mtMap1->IsDefined("MTName"));
}
TEST_F(aliLuaCoreMT, name) {
  ASSERT_STREQ(aNoDup->Name().c_str(), "mtTestObject1");
  ASSERT_STREQ(aDup->Name().c_str(), "mtTestObject2");
  ASSERT_STREQ(aSer->Name().c_str(), "mtTestObject3");
}
TEST_F(aliLuaCoreMT, getByindex) {
  Future::Ptr fPtr = Future::Create();
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(false));
  aDup  ->Register(exec);
  aNoDup->Register(exec);
  aSer  ->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      // The association of 1 object to
      // 3 different MT's is completely valid and
      // reasonable assuming the MT's funtions
      // properly handle the object that is associated
      // with them.
      Obj::Ptr oPtr(new Obj);
      aDup  ->MakeObject(L,oPtr);
      aNoDup->MakeObject(L,oPtr);
      aSer  ->MakeObject(L,oPtr);

      MT::Ptr aPtr = MT::GetMT(L,1,false);
      MT::Ptr bPtr = MT::GetMT(L,2,false);
      MT::Ptr cPtr = MT::GetMT(L,3,false);
      *a = aPtr.get()==aDup.get();
      *b = bPtr.get()==aNoDup.get();
      *c = cPtr.get()==aSer.get();
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a);
  ASSERT_TRUE(*b);
  ASSERT_TRUE(*c);
}
TEST_F(aliLuaCoreMT, getNonMTByIndex) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  Future::Ptr fPtr = Future::Create();
  Util::Run(exec, fPtr, [=](lua_State *L) {
      lua_pushinteger(L,1);
      //add push of different type      
      MT::Ptr mt1 = MT::GetMT(L,1,true); // non-object index
      MT::Ptr mt2 = MT::GetMT(L,2,true); // invalid index
      *a = !mt1; // should be nullptr
      *b = !mt2; // shoudl be nullptr
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a);
  ASSERT_TRUE(*b);
}
TEST_F(aliLuaCoreMT, getByName) {
  MT::Ptr a = MT::GetMT("mtTestObject1");
  MT::Ptr b = MT::GetMT("mtTestObject2");
  MT::Ptr c = MT::GetMT("mtTestObject3");
  ASSERT_EQ(a.get(),aNoDup.get());
  ASSERT_EQ(b.get(),aDup.get());
  ASSERT_EQ(c.get(),aSer.get());
}
TEST_F(aliLuaCoreMT, is) {
  IPtr rc(new int(5));
  BPtr shouldBeTrue(new bool(false));
  BPtr shouldBeFalse(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);      
      MT::TPtr ptr = oPtr;
      *rc            = aDup->MakeObject(L,ptr);
      *shouldBeTrue  = aDup->Is(L,-1);
      *shouldBeFalse = aNoDup->Is(L,-1);
      return 3;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*rc, 1) << "Make should have pushed 1 object";
  ASSERT_TRUE(*shouldBeTrue) << "the pushed object's Is";
  ASSERT_FALSE(*shouldBeFalse) << "a different object's Is";
}
TEST_F(aliLuaCoreMT, makeObjAndGet) {
  BPtr shouldBeTrue(new bool(false));
  IPtr iPtr(new int(0));
  DPtr dPtr(new double(0));
  IPtr rc(new int(0));
  const int    iVal = 2342;
  const double dVal = 24326.234;
  Future::Ptr fPtr = Future::Create();
  aDup->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      void *addr = oPtr.get();
      oPtr->iVal = iVal;
      oPtr->dVal = dVal;
      MT::TPtr ptr   = oPtr;
      *rc = aDup->MakeObject(L,ptr);
      oPtr.reset(); // make sure pushed object shares ownership of the pointer
      ptr = aDup->Get(L,-1,false); // should not throw
      *shouldBeTrue = ptr.get()==addr;
      // get values

      // Note, the use of the static cast is generally avoided
      // when using aliLuaCore::MT in conjunction with aliLuaCore::StaticObject
      // or aliLuaCore::Object.  Since Lua maintains a void* for user data
      // there will have to be casting somewhere, ensuring its as safe
      // as possible will lead to robust code bases, which is what
      // aliLuaCore::Object and aliLuaCore::StaticObject aim to do.
      oPtr = std::static_pointer_cast<Obj>(ptr);
      *iPtr = oPtr->iVal;
      *dPtr = oPtr->dVal;
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*rc,1);
  ASSERT_TRUE(*shouldBeTrue) << "verify the get's addr==orig";
  ASSERT_EQ(dVal, *dPtr);
  ASSERT_EQ(iVal, *iPtr);
}
TEST_F(aliLuaCoreMT, makeWeakObj) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(true));
  IPtr rc(new int(0));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      *rc = aDup->MakeWeakObject(L,oPtr);
      MT::Ptr aPtr = MT::GetMT(L,1,false);
      *a = aPtr.get()==aDup.get(); // should be true
      MT::TPtr tPtr = aDup->Get(L,-1,true);
      *b = !!tPtr; // should be true
      // release the pointers
      oPtr.reset();
      tPtr.reset();
      tPtr = aDup->Get(L,-1,true);
      *c = !!tPtr; // should be false
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*rc, 1);
  ASSERT_TRUE(*a);
  ASSERT_TRUE(*b);
  ASSERT_FALSE(*c);
}
TEST_F(aliLuaCoreMT, makeToWeak) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(false));
  BPtr d(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  Util::Run(exec, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      aDup->MakeObject(L,oPtr); // index 1-> strong
      aDup->ToWeak(L,-1);       // index 2-> weak
      oPtr.reset();
      MT::TPtr s = aDup->Get(L,1,true);
      MT::TPtr w = aDup->Get(L,2,true);
      *a = !!s;  // should be true
      *b = !!w; // should be true
      s.reset();
      w.reset();
      lua_remove(L,2); // remove w value (s should still hold pointer)
      lua_gc(L,LUA_GCCOLLECT,0);
      s = aDup->Get(L,1,true);
      w = aDup->Get(L,2,true);
      *c = !!s; // should be true
      *d = !!w; // should be false (value removed)
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a)  << "s should retrieve a value";
  ASSERT_TRUE(*b)  << "w should retrieve a value";
  ASSERT_TRUE(*c)  << "s should retrieve a value";
  ASSERT_FALSE(*d) << "w should not retrieve a value";
}
TEST_F(aliLuaCoreMT, makeWeakToStrong) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(false));
  BPtr d(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  Util::Run(exec, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      aDup->MakeWeakObject(L,oPtr); // index 1-> strong
      aDup->ToStrong(L,-1);         // index 2-> weak
      oPtr.reset();
      MT::TPtr w = aDup->Get(L,1,true);
      MT::TPtr s = aDup->Get(L,2,true);
      *a = !!w; // should be true
      *b = !!s; // should be true
      w.reset();
      s.reset();
      lua_remove(L,2); // remove s value (w should no longer hold pointer)
      lua_gc(L,LUA_GCCOLLECT,0);
      w = aDup->Get(L,1,true);
      s = aDup->Get(L,2,true);
      *c = !!w; // should be false (s was released)
      *d = !!s; // should be false (value removed)
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a)  << "w should retrieve a value";
  ASSERT_TRUE(*b)  << "s should retrieve a value";
  ASSERT_FALSE(*c) << "w should not retrieve a value";
  ASSERT_FALSE(*d) << "s should not retrieve a value";
}
TEST_F(aliLuaCoreMT, dup) {
  BPtr a(new bool(true));
  BPtr b(new bool(false));
  BPtr c(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  aNoDup->Register(exec);
  aSer  ->Register(exec);
  Util::Run(exec,fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      aDup  ->MakeObject(L,oPtr); // index 1
      aNoDup->MakeObject(L,oPtr); // index 2
      aSer  ->MakeObject(L,oPtr); // index 3
      *a = TestUtil::DidThrow([=](){ MakeFn val = MT::Dup(L,1); });
      *b = TestUtil::DidThrow([=](){ MakeFn val = MT::Dup(L,2); });
      *c = TestUtil::DidThrow([=](){ MakeFn val = MT::Dup(L,3); });
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_FALSE(*a);
  ASSERT_TRUE( *b);
  ASSERT_FALSE(*c);
}
TEST_F(aliLuaCoreMT, serializeDeserialize) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  aNoDup->Register(exec);
  aSer  ->Register(exec);
  Util::Run(exec,fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      aDup  ->MakeObject(L,oPtr); // index 1
      aNoDup->MakeObject(L,oPtr); // index 2
      aSer  ->MakeObject(L,oPtr); // index 3
      *a = TestUtil::DidThrow([=](){
	  std::ostringstream out;
	  Serializer s(out);
	  MT::Serialize(L, 1, s); // should throw
	});
      *b = TestUtil::DidThrow([=](){
	  std::ostringstream out;
	  Serializer s(out);
	  MT::Serialize(L, 2, s); // should throw
	});
      *c = TestUtil::DidThrow([=](){
	  std::ostringstream out;
	  Serializer s(out);
	  MT::Serialize(L, 3, s); // should not throw
	  std::istringstream in(out.str());
	  Deserializer d(in);
	  StackGuard g(L,1);
	  aSer->Deserialize(L, d);
	  THROW_IF(g.Diff()!=1, "Deserialize did not appear to push the value");
	  MT::TPtr ptr = aSer->Get(L,-1,false); // will throw if cannot extract
	});
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a);
  ASSERT_TRUE(*b);
  ASSERT_FALSE(*c);
}
TEST_F(aliLuaCoreMT, registerMT) {
  ASSERT_TRUE(true) << "indirectly checked elsewhere";
}
TEST_F(aliLuaCoreMT, setMT) {
  ASSERT_TRUE(true) << "indirectly checked elsewhere";
}
TEST_F(aliLuaCoreMT, canDup) {
  ASSERT_FALSE(aNoDup->CanDup());
  ASSERT_TRUE(aDup->CanDup());
  ASSERT_TRUE(aSer->CanDup());
}
TEST_F(aliLuaCoreMT, canSer) {
  ASSERT_FALSE(aNoDup->CanSerialize());
  ASSERT_FALSE(aDup->CanSerialize());
  ASSERT_TRUE(aSer->CanSerialize());
}
TEST_F(aliLuaCoreMT, isDefined) {
  ASSERT_TRUE(aNoDup->IsDefined("MakeTrue"));
  ASSERT_FALSE(aDup->IsDefined("MakeTrue"));
  ASSERT_FALSE(aSer->IsDefined("MakeTrue"));
}
TEST_F(aliLuaCoreMT, derived) {
  IPtr aCnt(new int(0));
  IPtr bCnt(new int(0));
  BPtr a(new bool(true));
  BPtr b(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup    ->Register(exec);
  aDerived->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new ObjDerived([=]() {++(*aCnt);},
				   [=]() {++(*bCnt);}));
      aDerived->MakeObject(L, oPtr);
      oPtr.reset();
      *a = TestUtil::DidThrow([=]() {
	  // should work
	  Obj::Ptr oPtr = std::static_pointer_cast<Obj>(aDup->Get(L, 1, false));
	});
      *b = TestUtil::DidThrow([=]() {
	  // should work
	  ObjDerived::Ptr oPtr = std::static_pointer_cast<ObjDerived>(aDerived->Get(L, 1, false));
	});
      lua_pop(L,lua_gettop(L));
      lua_gc(L,LUA_GCCOLLECT,0); // free any ref in lua
      oPtr.reset(); // should free instance of Obj1
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_EQ(*aCnt, 1);
  ASSERT_EQ(*bCnt, 1);
  ASSERT_FALSE(*a);
  ASSERT_FALSE(*b);
}

TEST_F(aliLuaCoreMT, verifyThrowOnBadGet) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(false));
  BPtr d(new bool(false));
  BPtr e(new bool(false));
  BPtr f(new bool(true));
  BPtr g(new bool(true));
  BPtr h(new bool(true));
  BPtr i(new bool(true));
  BPtr j(new bool(true));
  Future::Ptr fPtr = Future::Create();
  aDup  ->Register(exec);
  aNoDup->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      {
	StackGuard sg(L,2);
	Obj::Ptr oPtr(new Obj);
	aNoDup->MakeObject(L,oPtr);
	lua_pushinteger(L,1);
	// stack:
	// index 1: aNoDup object (not a aDup)
	// index 2: integer
	// index 3: none/invalid
	*a = TestUtil::DidThrow([=]() {
	    aDup->Get(L,1,false); // different object cannot convert
	  });
	*b = TestUtil::DidThrow([=]() {
	    aDup->Get(L,2,false); // int cannot convert
	  });
	*c = TestUtil::DidThrow([=]() {
	    aDup->Get(L,3,false); // None cannot convert
	  });
	*d = TestUtil::DidThrow([=]() {
	    aDup->Get(L,1,true); // different object cannot convert
	  });
	*e = TestUtil::DidThrow([=]() {
	    aDup->Get(L,2,true); // int cannot convert
	  });
	*f = TestUtil::DidThrow([=]() {
	    aDup->Get(L,3,true); // none cannot convert, but its treated as null
	  });
      }
      {
	StackGuard sg(L,1);
	Obj::Ptr oPtr; // uninitialized
	aDup->MakeObject(L,oPtr);
	aDup->MakeWeakObject(L,oPtr);
	lua_pushnil(L);
	// stack:
	// index 1: released aDup
	// index 2: released weak aDup
	// index 3: nil
	// index 4: none/invalid
	*g = TestUtil::DidThrow([=]() {
	    THROW_IF(aDup->Get(L,1,true), "expecting a null pointer");
	  });
	*h = TestUtil::DidThrow([=]() {
	    THROW_IF(aDup->Get(L,2,true), "expecting a null pointer");
	  });
	*i = TestUtil::DidThrow([=]() {
	    THROW_IF(aDup->Get(L,3,true), "expecting a null pointer");
	  });
	*j = TestUtil::DidThrow([=]() {
	    THROW_IF(aDup->Get(L,4,true), "expecting a null pointer");
	  });
      }
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
   ASSERT_TRUE(*a);
   ASSERT_TRUE(*b);
   ASSERT_TRUE(*c);
   ASSERT_TRUE(*d);
   ASSERT_TRUE(*e);
   ASSERT_FALSE(*f);
   ASSERT_FALSE(*g);
   ASSERT_FALSE(*h);
   ASSERT_FALSE(*i);
   ASSERT_FALSE(*j);
}

TEST_F(aliLuaCoreMT, verifyWeakDup) {
  BPtr a(new bool(false));
  BPtr b(new bool(false));
  BPtr c(new bool(false));
  Future::Ptr fPtr = Future::Create();
  aDup->Register(exec);
  Util::Run(exec, fPtr, [=](lua_State *L) {
      Obj::Ptr oPtr(new Obj);
      aDup->MakeWeakObject(L,oPtr);
      MakeFn mkFn = MT::Dup(L,-1); // presumably a weak duplication
      mkFn(L);                     // push another presumably weak reference
      oPtr.reset();                // release only strong pointer
      //
      // All pointers to oPtr's original object
      // should be weak or released at this point.
      // Even mkFn should also only contain a weak pointer.
      MT::TPtr t1Ptr = aDup->Get(L,1,true);
      MT::TPtr t2Ptr = aDup->Get(L,2,true);
      *a = !t1Ptr; // pointer should be null, assignment should be true
      *b = !t2Ptr; // pointer should be null, assignment should be true
      *c = TestUtil::DidThrow([=]() {
	  MT::TPtr t3Ptr = aDup->Get(L,2,false);
	});
      return 0;
    });
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(*a);
  ASSERT_TRUE(*b);
  ASSERT_TRUE(*c);
}

TEST_F(aliLuaCoreMT, scriptAPI_MTName) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_MTName"
		   "\n local obj = lib.mtTest.Create(4,43.2)"
		   "\n assert(obj:MTName()=='" MT_NAME "', 'bad obj::MTName()')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST_F(aliLuaCoreMT, scriptAPI_IVal) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_IVal"
		   "\n local obj = lib.mtTest.Create(4,43.2)"
		   "\n assert(obj:IVal()==4, 'bad obj::IVal() value')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST_F(aliLuaCoreMT, scriptAPI_DVal) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_DVal"
		   "\n local obj = lib.mtTest.Create(4,43.2)"
		   "\n assert(obj:DVal()==43.2, 'bad obj::DVal() value')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST_F(aliLuaCoreMT, scriptAPI_tostring) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_tostring"
		   "\n local obj = lib.mtTest.Create(4,43.2)"
		   "\n assert(type(tostring(obj)), 'bad tostrig(obj)')"
		   "\n assert(tostring(obj)=='" TO_STRING_TEXT "',"
		   "\n        'bad string output '..tostring(obj).."
		   "\n        ' does not equal " TO_STRING_TEXT "')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST_F(aliLuaCoreMT, scriptAPI_ToWeak) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_ToWeak"
		   "\n local obj  = lib.mtTest.Create(4,43.2)"
		   "\n local weak = obj:ToWeak()"
		   "\n assert(weak:IVal()==4, 'bad obj:IVal()')"
		   "\n obj = nil"
		   "\n collectgarbage()"
		   "\n assert(weak:IsValid()==false, 'bad obj:IsValid() or obj:ToWeak()')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

TEST_F(aliLuaCoreMT, scriptAPI_ToStrong) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_ToStrong"
		   "\n local obj  = lib.mtTest.Create(4,43.2)"
		   "\n local weak = obj:ToWeak()"
		   "\n assert(weak:IVal()==4, 'bad obj:IVal()')"
		   "\n local strong = obj:ToStrong()"
		   "\n obj = nil"
		   "\n collectgarbage()"
		   "\n assert(weak:IsValid()==true, 'bad obj:IsValid() or obj:ToStrong()')"
		   "\n assert(strong:IVal()==4, 'bad weak:ToStrong() or strong:IVal()')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

TEST_F(aliLuaCoreMT, scriptAPI_Release) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_Release"
		   "\n local obj  = lib.mtTest.Create(4,43.2)"
		   "\n obj:Release()"
		   "\n assert(obj:IsValid()==false, 'bad obj:Release() or obj:IsValid()')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

TEST_F(aliLuaCoreMT, scriptAPI_gc) {
  Future::Ptr fPtr = Future::Create();  
  Util::LoadString(exec,fPtr,
		   "-- scriptAPI_gc"
		   "\n local obj  = lib.mtTest.Create(4,43.2)"
		   "\n obj = nil"
		   "\n collectgarbage()"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
  ASSERT_EQ(Obj::Count(), 0u);
}
