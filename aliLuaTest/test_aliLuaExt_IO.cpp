#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <cstring>
#include <sstream>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using IO         = aliLuaExt::IO;
  using IOOptions  = aliLuaExt::IOOptions;
  using StackGuard = aliLuaCore::StackGuard;
  using Util       = aliLuaCore::Util;
  using IPtr       = aliLuaTest::Util::IPtr;
  using LPtr       = aliLuaTest::Util::LPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
  using SVec       = std::vector<std::string>;
  using IVec       = std::vector<int>;
  using DVec       = std::vector<double>;

  const SVec sVec({"ABC", "DEF", "HIJ", "KLM", "NOP"});
  const IVec iVec({11, 13, 17, 23, 29, 31, 37, 41});
  const DVec dVec({81.22435, -43.98432, 500.25749, 99.76325});
  const SVec tVec({"subTable1", "subTable2", "subTable3"});

  void LoadState(lua_State *L) {
    lua_checkstack(L,20);
    std::for_each(sVec.begin(), sVec.end(), [=](const std::string &val) {
	lua_pushstring(L,val.c_str());
      });
    std::for_each(iVec.begin(), iVec.end(), [=](int i) {
	lua_pushinteger(L,i);
      });
    std::for_each(dVec.begin(), dVec.end(), [=](double d) {
	lua_pushnumber(L,d);
      });
    lua_newtable(L);
    lua_pushvalue(L,-1);
    std::for_each(tVec.begin(), tVec.end(), [=](const std::string &) {
	lua_newtable(L);
      });
    std::for_each(tVec.rbegin(), tVec.rend(), [=](const std::string &tbl) {
	lua_setfield(L, -2, tbl.c_str());
      });
  }
  //   lua_newtable(L);
  //   lua_pushvalue(L,-2);
  //   lua_setfield(L, -2, "repeat");
  //   lua_setfield(L, -2, "recursiveSubTable");
  //   std::for_each(sVec.begin(), sVec.end(), [=](const std::string &val) {
  // 	lua_pushstring(L,val.c_str());
  //     });
    
  // }

  void LoadSearchItems(SVec &items, const SVec &in) {
    std::for_each(in.begin(), in.end(), [items=&items](const std::string &s) {
	items->push_back(s);
      });
  }
  void LoadSearchItems(SVec &items, const IVec &in) {
    std::for_each(in.begin(), in.end(), [items=&items](int i) {
	char iBuf[25];
	snprintf(iBuf, sizeof(iBuf), "%i", i);
	items->push_back(iBuf);
      });
  }
  void LoadSearchItems(SVec &items, const DVec &in) {
    std::for_each(in.begin(), in.end(), [items=&items](double d) {
	char dBuf[25];
	snprintf(dBuf, sizeof(dBuf), "%6.3lf", d);
	items->push_back(dBuf);
      });
  }


}


TEST(aliLuaExtIO, singleIndex) {
  IPtr       iPtr(new int(0));
  LPtr       lPtr = TestUtil::GetL();
  lua_State *L    = lPtr.get();
  SVec       items;
  LoadState(L);
  LoadSearchItems(items, sVec);
  LoadSearchItems(items, iVec);
  LoadSearchItems(items, dVec);
  int top = lua_gettop(L);
  std::for_each(items.begin(), items.end(), [=](const std::string &s) {
      std::stringstream ss;
      ss << IO(L,++(*iPtr));
      std::string str = ss.str();
      ASSERT_TRUE(std::strstr(str.c_str(), s.c_str()));
    });
  items.clear();
  LoadSearchItems(items, tVec);
  std::stringstream ss;
  ss << IO(L, ++(*iPtr));
  std::string str = ss.str();
  std::for_each(items.begin(), items.end(), [=](const std::string &s) {
      ASSERT_TRUE(std::strstr(str.c_str(), s.c_str()));
    });
  ASSERT_EQ(top, lua_gettop(L)) << "verify stack size remained constant";
}

TEST(aliLuaExtIO, allIndices) {
  LPtr       lPtr = TestUtil::GetL();
  lua_State *L    = lPtr.get();
  SVec       items;
  LoadState(L);
  LoadSearchItems(items, sVec);
  LoadSearchItems(items, iVec);
  LoadSearchItems(items, dVec);
  LoadSearchItems(items, tVec);
  int top = lua_gettop(L);
  std::stringstream ss;
  ss << IO(L);
  const std::string str = ss.str();
  std::for_each(items.begin(), items.end(), [=](const std::string &s) {
      ASSERT_TRUE(std::strstr(str.c_str(), s.c_str()));
    });
  ASSERT_EQ(top, lua_gettop(L)) << "verify stack size remained constant";
}

TEST(aliLuaExtIO, indexRange) {
  LPtr       lPtr = TestUtil::GetL();
  lua_State *L    = lPtr.get();
  SVec       items;
  size_t     cnt = 4;
  LoadState(L);
  LoadSearchItems(items, sVec);
  LoadSearchItems(items, iVec);
  LoadSearchItems(items, dVec);
  size_t limit = items.size();
  LoadSearchItems(items, tVec);
  int top = lua_gettop(L);
  for (size_t i=0; i<limit-cnt;++i) {
    std::stringstream ss;
    IOOptions opt(1+i, cnt);
    ss << IO(L,opt);
    const std::string str = ss.str();
    for (SVec::const_iterator it=items.begin(); it!=items.end(); ++it) {
      size_t pos = it-items.begin();
      if (pos>=i && pos<i+cnt) {
	ASSERT_TRUE(std::strstr(str.c_str(), it->c_str()));
      } else {
	ASSERT_FALSE(std::strstr(str.c_str(), it->c_str()));
      }
    }
  }
  ASSERT_EQ(top, lua_gettop(L)) << "verify stack size remained constant";
}

TEST(aliLuaExtIO, separators) {
  char buf[25];
  const int   i1  = 13;
  const int   i2  = 23;
  const char *sep = "--|--";
  LPtr        lPtr = TestUtil::GetL();
  lua_State  *L    = lPtr.get();
  lua_pushinteger(L,i1);
  lua_pushinteger(L,i2);
  std::stringstream ss;
  IOOptions opt;
  opt.SetSeparator(sep);
  ss << IO(L,opt);
  snprintf(buf, sizeof(buf), "%i%s%i", i1, sep, i2);
  std::string str = ss.str();
  ASSERT_STREQ(str.c_str(), buf);
}

TEST(aliLuaExtIO, indentAndNewLines) {
  LPtr        lPtr = TestUtil::GetL();
  lua_State  *L    = lPtr.get();
  std::string withNewLines;
  std::string withoutNewLines;
  lua_newtable(L);
  lua_pushinteger(L,13);
  lua_setfield(L,-2,"iVal");
  {
    std::stringstream ss;
    IOOptions opt;
    opt.SetIndentSize(4);
    ss << IO(L,opt);
    withNewLines = ss.str();
  }
  {
    std::stringstream ss;
    IOOptions opt;
    opt.SetIndentSize(4);
    opt.SetEnableNewLines(false);
    ss << IO(L,opt);
    withoutNewLines = ss.str();
  }
  ASSERT_TRUE(nullptr!=std::strstr(withNewLines.c_str(),    "    "));
  ASSERT_TRUE(nullptr==std::strstr(withoutNewLines.c_str(), "    "));
  ASSERT_TRUE(nullptr!=std::strstr(withNewLines.c_str(),    "\n"));
  ASSERT_TRUE(nullptr==std::strstr(withoutNewLines.c_str(), "\n"));
  withNewLines.erase(std::remove(withNewLines.begin(), withNewLines.end(), '\n'), withNewLines.end());
  withNewLines.erase(std::remove(withNewLines.begin(), withNewLines.end(), ' '), withNewLines.end());
  withoutNewLines.erase(std::remove(withoutNewLines.begin(), withoutNewLines.end(), ' '), withoutNewLines.end());
  ASSERT_STREQ(withNewLines.c_str(), withoutNewLines.c_str());
}

TEST(aliLuaExtIO, withTableAddress) {
  LPtr       lPtr = TestUtil::GetL();
  lua_State *L    = lPtr.get();
  lua_newtable(L);
  lua_pushinteger(L,13);
  lua_setfield(L,-2,"iVal");
  {
    std::stringstream ss;
    IOOptions         opt;
    std::string       str;
    opt.SetShowTableAddress(true);
    ss << IO(L,opt);
    str = ss.str();
    ASSERT_TRUE(nullptr!=std::strstr(str.c_str(), "0x"));
  }
  {
    std::stringstream ss;
    IOOptions         opt;
    std::string       str;
    opt.SetShowTableAddress(false);
    ss << IO(L,opt);
    str = ss.str();
    ASSERT_TRUE(nullptr==std::strstr(str.c_str(), "0x"));
  }
}

TEST(aliLuaExtIO, serialize) {
  LPtr               lPtr = TestUtil::GetL();
  lua_State         *L    = lPtr.get();
  std::stringstream ss;
  IOOptions         opt;
  lua_pushinteger(L,13);
  lua_pushstring(L,"some string");
  lua_pushnumber(L, 4.25);
  opt.SetSerialize(aliSystem::BasicCodec::GetSerializer());
  ss << IO(L,opt);
  aliSystem::Codec::Deserializer d(ss);
  int rc = aliLuaCore::Deserialize::ToLua(L, d);
  ASSERT_EQ(rc, 3);
  ASSERT_EQ(   lua_tonumber(L,1), lua_tonumber(L,4));
  ASSERT_STREQ(lua_tostring(L,2), lua_tostring(L,5));
  ASSERT_EQ(   lua_tonumber(L,3), lua_tonumber(L,6));
  ASSERT_EQ(   lua_gettop(L),     6);
}

TEST(aliLuaExtIO, scriptSerialize) {
  const std::string name = "Lua IO - scriptSerialize";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- ioTest scriptLog"
		   "\n local serialize   = lib.aliLua.codec.GetBasicSerialize()"
		   "\n local deserialize = lib.aliLua.codec.GetBasicDeserialize()"
		   "\n local str = serialize:Serialize('some string', 342, 4.6, { x=123, {3,5}})"
		   "\n local rtn = {deserialize:Deserialize(str)}"
		   "\n assert(rtn[1]=='some string',    'bad serialization/deserialization')"
		   "\n assert(rtn[2]==342,              'bad serialization/deserialization')"
		   "\n assert(rtn[3]==4.6,              'bad serialization/deserialization')"
		   "\n assert(type(rtn[4])=='table',    'bad serialization/deserialization')"
		   "\n assert(rtn[4].x==123,            'bad serialization/deserialization')"
		   "\n assert(type(rtn[4][1])=='table', 'bad serialization/deserialization')"
		   "\n assert(rtn[4][1][1]==3,          'bad serialization/deserialization')"
		   "\n assert(rtn[4][1][2]==5,          'bad serialization/deserialization')"
		   "\n assert(rtn[5]==nil,              'bad serialization/deserialization')"
		   );
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
TEST(aliLuaExtIO, scriptToString) {
  const std::string name = "Lua IO - scriptToString";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- ioTest scriptLog"
		   "\n function Find(src, str)"
		   "\n    return not not src:find(str, 1, true)"
		   "\n end"
		   "\n local opt = {"
		   "\n    indentSize       =      2,"
		   "\n    separator        =   '##',"
		   "\n    rootName         = 'ROOT',"
		   "\n    enableNewLines   =   true,"
		   "\n    showTableAddress =  false,"
		   "\n    serialize        =  nil,"
		   "\n }"
		   "\n local serialize   = lib.aliLua.codec.GetBasicSerialize()"
		   "\n local deserialize = lib.aliLua.codec.GetBasicDeserialize()"
		   "\n local str = lib.aliLua.io.ToString(opt, 'some string', 342, 4.6, { x=123, {3,5}})"
		   "\n assert(    Find(str, '    '),  '1 - indentSize error')"
		   "\n assert(not Find(str, '     '), '2 - indentSize error')"
		   "\n assert(Find(str, '\\n'), '3 - enableNewLines error')"
		   "\n assert(Find(str, '##'), '4 - separator error')"
		   "\n assert(not Find(str, '0x'), '5 - showTableAddress error')"
		   "\n opt.indentSize = 3"
		   "\n opt.showTableAddress = true"
		   "\n str = lib.aliLua.io.ToString(opt, 'some string', 342, 4.6, { x=123, {3,5}})"
		   "\n assert(Find(str, '0x'), '6 - showTableAddress error')"
		   "\n assert(Find(str, '     '), '7 - indentSize error')"
		   "\n opt.enableNewLines = false"
		   "\n str = lib.aliLua.io.ToString(opt, 'some string', 342, 4.6, { x=123, {3,5}})"
		   "\n assert(not Find(str, '\\n'), '8 - enableNewLines error')"
		   "\n assert(not Find(str, '  '), '9 - enableNewLines error')"
		   "\n local subTbl = {}"
		   "\n subTbl[1] = subTbl"
		   "\n local tbl = {subTbl, subTbl}"
		   "\n opt.showTableAddress = false"
		   "\n str = lib.aliLua.io.ToString(opt, tbl)"
		   "\n assert(Find(str, 'ROOT'), '10 - rootName')"
		   "\n str = lib.aliLua.io.ToString({serialize=serialize}, 34.2, 'xyz', 400)"
		   "\n local rtn = {deserialize:Deserialize(str)}"
		   "\n assert(rtn[1]==34.2,  '11 - deserialize')"
		   "\n assert(rtn[2]=='xyz', '12 - deserialize')"
		   "\n assert(rtn[3]==400,   '13 - deserialize')"
		   "\n assert(rtn[4]==nil,   '14 - deserialize')"
		   "\n ");
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}
TEST(aliLuaExtIO, scriptLog) {
  // just verifying interface exists and doesn't blow up
  const std::string name = "Lua IO - scriptLog";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- ioTest scriptLog"
		   "\n lib.aliLua.io.Log('some string', 342, 4.6, { x=123, {3,5}})");
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}

