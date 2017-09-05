#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaTest_util.hpp>
#include <lua.hpp>

namespace {
  using Deserialize  = aliLuaCore::Deserialize;
  using Deserializer = aliSystem::Codec::Deserializer;
  using MakeFn       = aliLuaCore::MakeFn;
  using Serialize    = aliLuaCore::Serialize;
  using Serializer   = aliSystem::Codec::Serializer;
  using StackGuard   = aliLuaCore::StackGuard;
  using Table        = aliLuaCore::Table;
  using Values       = aliLuaCore::Values;
  using TestUtil     = aliLuaTest::Util;
  
  struct aliLuaCoreTable : testing::Test {
    std::string key;
    const int   tableIndex = 1;
    const int   strIndex   = 2;
    const int   intIndex   = 3;
    const int   nilIndex   = 4;
    const int   undefined  = 5;
    using SGPtr = std::unique_ptr<StackGuard>;
    void SetUp() {
      key = "itemKey";
      sg.reset(new StackGuard(L,5));
      lua_newtable(L);
      lua_pushstring(L,"junk");
      lua_pushinteger(L,5); // more junk
      lua_pushnil(L);
    }
    void TearDown() {
      sg.reset();
    }
    static void SetUpTestCase() {
      L = luaL_newstate();
    }
    static void TearDownTestCase() {
      lua_close(L);
    }
    static lua_State *L;
  private:
    SGPtr sg;
  };
  lua_State *aliLuaCoreTable::L = nullptr;

}

TEST_F(aliLuaCoreTable, DefaultValues) {
  int            iVal;
  double         dVal;
  std::string    sVal;
  bool           bVal;
  MakeFn mVal;
  Table::GetInteger(L,tableIndex, "key", iVal, true, 43);
  Table::GetDouble (L,tableIndex, "key", dVal, true, 100.3);
  Table::GetString (L,tableIndex, "key", sVal, true, "def-value");
  Table::GetBool   (L,tableIndex, "key", bVal, true, true);
  Table::GetMakeFn (L,tableIndex, "key", mVal, true, Values::GetMakeIntegerFn(33));
  ASSERT_EQ(iVal, 43);
  ASSERT_EQ(dVal, 100.3);
  ASSERT_EQ(sVal, "def-value");
  ASSERT_TRUE(bVal);
  int rc = mVal(L);
  ASSERT_EQ(rc,1) << "Make sure 1 value is pushed";
  ASSERT_EQ(33, lua_tointeger(L,-1));
}

TEST_F(aliLuaCoreTable, Nil) {
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::SetInteger(L,tableIndex, key, 5);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::SetNil(L,tableIndex, key);
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
}


TEST_F(aliLuaCoreTable, Integer) {
  const int val=21;
  int res=-1;
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::GetInteger(L, tableIndex, key, res, true);
  ASSERT_EQ(res, 0); // default value should be set
  bool didThrow = TestUtil::DidThrow([=]() {
      int res = -1;
      Table::GetInteger(L,tableIndex, key, res, false);
    });
  ASSERT_TRUE(didThrow);
  Table::SetInteger(L,tableIndex, key, val);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::GetInteger(L,tableIndex, key, res, true);
  ASSERT_EQ(res, val);
}
TEST_F(aliLuaCoreTable, Double) {
  const double val=4.1;
  double res=-1.1;
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::GetDouble(L, tableIndex, key, res, true);
  ASSERT_EQ(res, 0); // default value should be set
  bool didThrow = TestUtil::DidThrow([=]() {
      double res = -1;
      Table::GetDouble(L,tableIndex, key, res, false);
    });
  ASSERT_TRUE(didThrow);
  Table::SetDouble(L,tableIndex, key, val);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::GetDouble(L,tableIndex, key, res, true);
  ASSERT_EQ(res, val);
}
TEST_F(aliLuaCoreTable, String) {
  const std::string val="string val";
  std::string res="string not val";
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::GetString(L, tableIndex, key, res, true);
  ASSERT_STREQ(res.c_str(), ""); // default value should be set
  bool didThrow = TestUtil::DidThrow([=]() {
      std::string res = "string not val";
      Table::GetString(L,tableIndex, key, res, false);
    });
  ASSERT_TRUE(didThrow);
  Table::SetString(L,tableIndex, key, val);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::GetString(L,tableIndex, key, res, true);
  ASSERT_STREQ(res.c_str(), val.c_str());
}
TEST_F(aliLuaCoreTable, Bool) {
  const bool val=true;
  bool res=false;
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::GetBool(L, tableIndex, key, res, true);
  ASSERT_EQ(res, false); // default value should be set
  bool didThrow = TestUtil::DidThrow([=]() {
      bool res = false;
      Table::GetBool(L,tableIndex, key, res, false);
    });
  ASSERT_TRUE(didThrow);
  Table::SetBool(L,tableIndex, key, val);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::GetBool(L,tableIndex, key, res, true);
  ASSERT_EQ(res, val);
}
TEST_F(aliLuaCoreTable, MakeFn) {
  bool didThrow;
  const int iVal = 9;
  StackGuard g(L,5);
  const MakeFn val = Values::GetMakeIntegerFn(iVal);
  MakeFn res;
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  aliLuaCore::Table::SetInteger(L,tableIndex, key, 2634);
  didThrow = TestUtil::DidThrow([=]() {
      Table::SetMakeFn(L,tableIndex, key, Values::MakeNothing);
    });
  ASSERT_FALSE(didThrow);
  ASSERT_TRUE(Table::IsNil(L,tableIndex, key)) << "a 'make nothing' make function"
    " should nil out the table key";
  Table::GetMakeFn(L, tableIndex, key, res, true);
  res(L);
  ASSERT_EQ(g.Diff(),0) << "default MakeFn should be 'MakeNothing'";
  //
  //
  didThrow = TestUtil::DidThrow([=]() {
      MakeFn res;
      Table::GetMakeFn(L,tableIndex, key, res, false);
    });
  ASSERT_TRUE(didThrow);
  Table::SetMakeFn(L,tableIndex, key, val);
  ASSERT_FALSE(Table::IsNil(L, tableIndex, key));
  Table::GetMakeFn(L,tableIndex, key, res, true);
  res(L);
  ASSERT_EQ(lua_tointeger(L,-1),iVal);
}
TEST_F(aliLuaCoreTable, SerializeValue) {
  bool                   didThrow;
  std::string            val  = "junk";
  int                    i1   = 95;
  int                    i2   = 43;
  int                    i3   = 999;
  std::stringstream      twoI;
  StackGuard g(L,5);
  //
  // build a serialized string with 2 values to help testing
  Serializer sObj1(twoI);
  lua_pushinteger(L,i1);
  lua_pushinteger(L,i2);
  Serialize::Write(L, -2, 2, sObj1);
  std::string twoIStr = twoI.str();
  g.Clear();
  //
  // start testing
  std::stringstream out2;
  Serializer sObj2(out2);
  ASSERT_TRUE(Table::IsNil(L, tableIndex, key));
  Table::GetSerializedValue(L,tableIndex, key, sObj2, true);
  ASSERT_EQ(g.Diff(),0) << "stack should be unchanged";
  Deserializer dObj2(out2);
  Deserialize::ToLua(L, dObj2);
  ASSERT_EQ(g.Diff(),1) << "1 value (nil) should have been pushed on the stack";
  ASSERT_TRUE(lua_isnil(L,-1)) << "top value should be nil";
  g.Clear();
  didThrow = TestUtil::DidThrow([=]() {
      std::stringstream ss(twoIStr);
      Deserializer dObj3(ss);
      Table::SetSerializedValue(L,tableIndex, key, dObj3);
    });
  ASSERT_TRUE(didThrow) << "Should have thrown if more than 1 value is"
    " pushed by deserializing the string";
  didThrow = TestUtil::DidThrow([=]() {
      std::stringstream out3;
      Serializer sObj3(out3);
      Table::GetSerializedValue(L,tableIndex, key, sObj3, false);
    });
  ASSERT_TRUE(didThrow) << "Should have thrown if a the table value is nil"
    " and allowNil==false";
  ASSERT_EQ(g.Diff(), 0) << "stack should be unchanged";

  //
  //
  std::stringstream out4;
  Serializer sObj4(out4);
  Table::SetInteger(L,tableIndex, key, i3);
  Table::GetSerializedValue(L,tableIndex, key, sObj4, true);
  Deserializer dObj4(out4);
  Deserialize::ToLua(L, dObj4);
  ASSERT_EQ(g.Diff(),1);
  int i4 = lua_tointeger(L,-1);
  ASSERT_EQ(i3, i4);
}
TEST_F(aliLuaCoreTable, GetSequence) {
  int                i1 =   5;
  int                i2 =  62;
  int                i3 = 937;
  MakeFn     seq;
  StackGuard g(L,4);
  Table::GetSequence(L,tableIndex, key, seq, true);
  seq(L);
  ASSERT_EQ(g.Diff(),0);
  //
  // verify GetSequence throws if a key holds something other than a table
  bool didThrow = TestUtil::DidThrow([=]() {
      Table::SetInteger(L,tableIndex,key,100);
      MakeFn seq;
      Table::GetSequence(L, tableIndex, key, seq, true);
    });
  ASSERT_TRUE(didThrow) << "Should throw if value is not a table";
  Table::SetNil(L,tableIndex,key);

  //
  // verify GetSequence does not throw for nil values
  bool nothingPushed = false;
  didThrow = TestUtil::DidThrow([L=L,
				 tableIndex=tableIndex,
				 key=key,
				 nothingPushed=&nothingPushed]() {
				  StackGuard g(L,1);
				  MakeFn seq;
				  Table::GetSequence(L, tableIndex, key, seq, true);
				  seq(0);
				  *nothingPushed = g.Diff()==0;
				});
  ASSERT_FALSE(didThrow) << "Should no throw if nil & allowing nil";
  ASSERT_TRUE(nothingPushed) << "Nothing should be pushed";
  Table::SetNil(L,tableIndex,key);

  //
  // fill table key with a sequence & verify extraction
  lua_createtable(L,3,0);
  lua_pushinteger(L,i1);
  lua_pushinteger(L,i2);
  lua_pushinteger(L,i3);
  lua_rawseti(L, g.Index(1), 3);
  lua_rawseti(L, g.Index(1), 2);
  lua_rawseti(L, g.Index(1), 1);
  ASSERT_EQ(g.Diff(),1);
  lua_setfield(L,tableIndex,key.c_str());
  ASSERT_EQ(g.Diff(),0);
  Table::GetSequence(L, tableIndex, key, seq, false);
  ASSERT_EQ(g.Diff(),0);
  seq(L);
  ASSERT_EQ(g.Diff(),3);
  ASSERT_EQ(lua_tointeger(L,-3),i1);
  ASSERT_EQ(lua_tointeger(L,-2),i2);
  ASSERT_EQ(lua_tointeger(L,-1),i3);
}
void StringItems(size_t cnt, std::function<void(const char*buf, size_t i)> fn) {
  for (size_t i=0;i<cnt;++i) {
    char buf[25];
    snprintf(buf, sizeof(buf), "str_%zi", i);
    fn(buf, i);
  }
}
TEST_F(aliLuaCoreTable, StringSequenceAsSet) {
  StackGuard  g(L,2);
  Table::SSet sSetSrc;
  Table::SSet sSet;
  Table::SSet sSet2;
  Table::GetStringSequence(L,tableIndex, "junk", sSet);
  ASSERT_TRUE(sSet.empty());
  ASSERT_EQ(g.Diff(),0);
  lua_newtable(L);
  StringItems(5,[&](const char *buf, size_t i) {
      lua_pushstring(L,buf);
      lua_rawseti(L,g.Index(1),(int)i+1);
      sSetSrc.insert(buf);
    });
  lua_setfield(L,tableIndex, key.c_str());
  ASSERT_EQ(g.Diff(),0);
  //
  // extract items (join with current items in set)
  sSet.insert("dummy");
  Table::GetStringSequence(L,tableIndex, key, sSet);
  ASSERT_EQ(g.Diff(),0);
  ASSERT_EQ(sSet.size(),1+5u) << "Make sure the original item and new items are there";
  StringItems(5,[&](const char *buf, size_t) {
      ASSERT_TRUE(sSet.find(buf)!=sSet.end()) << "Didn't find the value " << buf;
    });
  Table::SetStringSequence(L,tableIndex, "newSeq", sSet);
  ASSERT_EQ(g.Diff(),0);
  lua_getfield(L,tableIndex,"newSeq");
  ASSERT_EQ(g.Diff(),1);
  ASSERT_TRUE(lua_istable(L,g.Index(1)));
  lua_pushnil(L);
  while (lua_next(L,g.Index(1))) {
    sSet2.insert(lua_tostring(L,-1));
    lua_pop(L,1);
  }
  ASSERT_EQ(sSet.size(),sSet2.size());
  for (Table::SSet::iterator it=sSet.begin(); it!=sSet.end();++it) {
    ASSERT_TRUE(sSet2.find(*it)!=sSet2.end());
  }
}
TEST_F(aliLuaCoreTable, StringSequenceAsVec) {
  StackGuard  g(L,2);
  Table::SVec sVecSrc;
  Table::SVec sVec;
  Table::GetStringSequence(L,tableIndex, "junk", sVec);
  ASSERT_TRUE(sVec.empty());
  ASSERT_EQ(g.Diff(),0);
  lua_newtable(L);
  StringItems(5,[&](const char *buf, size_t i) {
      lua_pushstring(L,buf);
      lua_rawseti(L,g.Index(1),(int)i+1);
      sVecSrc.push_back(buf);
    });
  lua_setfield(L,tableIndex, key.c_str());
  ASSERT_EQ(g.Diff(),0);
  //
  // extract items (join with current items in set)
  sVec.push_back("dummy");
  Table::GetStringSequence(L,tableIndex, key, sVec);
  ASSERT_EQ(g.Diff(),0);
  ASSERT_EQ(sVec.size(),1+5u) << "Make sure the original item and new items are there";
  StringItems(5,[&](const char *buf, size_t i) {
      ASSERT_STREQ(sVec[i+1].c_str(), buf);
    });
  Table::SetStringSequence(L,tableIndex, "newSeq", sVec);
  ASSERT_EQ(g.Diff(),0);
  lua_getfield(L,tableIndex,"newSeq");
  ASSERT_EQ(g.Diff(),1);
  ASSERT_TRUE(lua_istable(L,g.Index(1)));
  lua_pushnil(L);
  size_t idx=0;
  while (lua_next(L,g.Index(1))) {
    ASSERT_STREQ(lua_tostring(L,-1), sVec[idx++].c_str());
    lua_pop(L,1);
  }
}

TEST_F(aliLuaCoreTable, VerifyBadTableIndexThrows) {
  using Fn    = std::function<void(int)>;
  using FnVec = std::vector<Fn>;
  FnVec fnVec;
  fnVec.push_back([=](int index){
      Table::IsNil (L,index,key);
    });
  fnVec.push_back([=](int index){
      Table::SetNil(L,index,key);
    });
  fnVec.push_back([=](int index){
      int i=0;
      Table::GetInteger(L,index,key,i,true);
    });
  fnVec.push_back([=](int index){
      Table::SetInteger(L,index,key,5);
    });
  fnVec.push_back([=](int index){
      double d=0;
      Table::GetDouble(L,index,key,d,true);
    });
  fnVec.push_back([=](int index){
      Table::SetDouble(L,index,key,4.3);
    });
  fnVec.push_back([=](int index){
      std::string s;
      Table::GetString(L,index,key,s,true);
    });
  fnVec.push_back([=](int index){
      Table::SetString(L,index,key, "junk");
    });
  fnVec.push_back([=](int index){
      bool b = false;
      Table::GetBool(L,index,key,b,true);
    });
  fnVec.push_back([=](int index){
      Table::SetBool(L,index,key,true);
    });
  fnVec.push_back([=](int index){
      MakeFn m;
      Table::GetMakeFn(L,index,key,m,true);
    });
  fnVec.push_back([=](int index){
      std::stringstream ss;
      Serializer v(ss);
      Table::GetSerializedValue(L,index,key,v,true);
    });
  fnVec.push_back([=](int index){
      MakeFn m;
      Table::GetSequence(L,index,key,m,true);
    });
  fnVec.push_back([=](int index){
      Table::SSet sSet;
      Table::GetStringSequence(L,index,key, sSet);
    });
  fnVec.push_back([=](int index){
      Table::SVec sVec;
      Table::GetStringSequence(L,index,key, sVec);
    });
  fnVec.push_back([=](int index){
      Table::SSet sSet;
      Table::SetStringSequence(L,index,key, sSet);
    });
  fnVec.push_back([=](int index){
      Table::SVec sVec;
      Table::SetStringSequence(L,index,key, sVec);
    });

  for (int index = 2; index<=undefined; ++index) {
    for (FnVec::iterator it=fnVec.begin(); it!=fnVec.end(); ++it) {
      bool didError = false;
      try {
	Fn fn = *it;
	fn(index);
      } catch (std::exception &e) {
	didError = true;
      }
      ASSERT_TRUE(didError);
    }
  }
}

