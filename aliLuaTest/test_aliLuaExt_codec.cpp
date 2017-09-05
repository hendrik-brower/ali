#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using Util       = aliLuaCore::Util;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaExt_codec, basicSerialize) {
  Pool::Ptr       pool   = Pool::Create("pool", 1);
  ExecEngine::Ptr engine = ExecEngine::Create("execEngine", pool);
  Future::Ptr     fPtr = Future::Create();
  Util::LoadString(engine, fPtr, ""
		   "-- test aliLuaExec::Codec - basicSerialize"
		   "\n local bs = lib.aliLua.codec.GetBasicSerialize()"
		   "\n assert(bs, 'failed to get a serialize object')"
		   "\n local info = bs:GetInfo()"
		   "\n assert(info.name=='basicCodec', 'bad name')"
		   "\n assert(info.version==1, 'bad version')"
		   "");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}
TEST(aliLuaExt_codec, basicDeserialize) {
  Pool::Ptr       pool   = Pool::Create("pool", 1);
  ExecEngine::Ptr engine = ExecEngine::Create("execEngine", pool);
  Future::Ptr     fPtr = Future::Create();
  Util::LoadString(engine, fPtr, ""
		   "-- test aliLuaExec::Codec - basicDeserialize"
		   "\n local bd = lib.aliLua.codec.GetBasicDeserialize()"
		   "\n assert(bd, 'failed to get a deserialize object')"
		   "\n local info = bd:GetInfo()"
		   "\n assert(info.name=='basicCodec', 'bad name')"
		   "\n assert(info.version==1, 'bad version')"
		   "\n for k,v in ipairs {"
		   "\n    { val =  true, name = 'basicCodec', ver = 0 },"
		   "\n    { val =  true, name = 'basicCodec', ver = 1 },"
		   "\n    { val = false, name = 'basicCodec', ver = 2 },"
		   "\n    { val = false, name = 'BAD_CODEC',  ver = 1 },"
		   "\n } do"
		   "\n    local err = string.format('bad can check %s %i',"
		   "\n                              v.name,"
		   "\n                              v.ver)"
		   "\n    assert(v.val==bd:CanDeserialize(v.name,v.ver), err)"
		   "\n end"
		   "");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

TEST(aliLuaExt_codec, serializeAndDeserialize) {
  Pool::Ptr       pool   = Pool::Create("pool", 1);
  ExecEngine::Ptr engine = ExecEngine::Create("execEngine", pool);
  Future::Ptr     fPtr = Future::Create();
  Util::LoadString(engine, fPtr, ""
		   "-- test aliLuaExec::Codec - serializeAndDeserialize"
		   "\n function Verify(key,v1,v2)"
		   "\n    assert(type(v1)==type(v2), 'bad types for '..key)"
		   "\n    local t = type(v1)"
		   "\n    if t=='string' or t=='number' or t=='boolean' then"
		   "\n       local err = string.format('bad match for %s - %s~=%s',"
		   "\n                                 key,"
		   "\n                                 tostring(v1),"
		   "\n                                 tostring(v2))"
		   "\n       assert(v1==v2, err)"
		   "\n    elseif t=='table' then"
		   "\n       for k,v in pairs(v1) do"
		   "\n           local newKey = string.format('%s.%s', key, tostring(k))"
		   "\n           Verify(newKey, v,v2[k])"
		   "\n       end"
		   "\n       for k,v in pairs(v2) do"
		   "\n           local newKey = string.format('%s.%s', key, tostring(k))"
		   "\n           Verify(newKey, v,v1[k])"
		   "\n       end"
		   "\n    else"
		   "\n       error('bad test for '..key)"
		   "\n    end"
		   "\n end"
		   "\n local bs = lib.aliLua.codec.GetBasicSerialize()"
		   "\n local bd = lib.aliLua.codec.GetBasicDeserialize()"
		   "\n local args = {"
		   "\n                 'abc',"
		   "\n                 53,"
		   "\n                 243.5,"
		   "\n                 true,"
		   "\n                 false,"
		   "\n                 {"
		   "\n                    'more junk',"
		   "\n                    {"
		   "\n                       'and even more'"
		   "\n                    }"
		   "\n                 }"
		   "\n              }"
		   "\n local str = bs:Serialize(table.unpack(args))"
		   "\n local rtn = {bd:Deserialize(str)}"
		   "\n Verify('root', args,rtn)"
		   "");
  TestUtil::Wait(engine, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

