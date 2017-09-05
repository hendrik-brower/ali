#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaExt3.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using Exec       = aliLuaCore::Exec;
  using ExecEngine = aliLuaExt::ExecEngine;
  using ExternalMT = aliLuaExt3::ExternalMT;
  using Future     = aliLuaCore::Future;
  using Util       = aliLuaCore::Util;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaExtExternalMT, general) {
  const std::string mtName = "myJunkMT";
  const std::string script =
    "-- test ExternalMT"
    "\n local args = nil"
    "\n function Create(...)"
    "\n    args = {...}"
    "\n end"
    "\n function Destroy(...)"
    "\n end"
    "\n function Fn1(...)"
    "\n    return args"
    "\n end"
    "\n function Fn2(...)"
    "\n    return 'fn2'"
    "\n end"
    ;
  const std::string createFn      = "Create";
  const std::string onDestroyFn   = "Destroy";
  ExternalMT::SSet functions;
  functions.insert("Fn1");
  functions.insert("Fn2");
  ExternalMT::Ptr ptr = ExternalMT::Create(mtName,
					   script,
					   createFn,
					   onDestroyFn,
					   functions);
  ASSERT_STREQ(mtName       .c_str(), ptr->MTName       ().c_str());
  ASSERT_STREQ(script       .c_str(), ptr->Script       ().c_str());
  ASSERT_STREQ(createFn     .c_str(), ptr->CreateFn     ().c_str());
  ASSERT_STREQ(onDestroyFn  .c_str(), ptr->OnDestroyFn  ().c_str());
  functions = ptr->Functions();
  ASSERT_EQ(functions.size(), 2u);
  ASSERT_TRUE(functions.find("Fn1")!=functions.end());
  ASSERT_TRUE(functions.find("Fn2")!=functions.end());
  ASSERT_TRUE(ptr->IsDefined("Fn1"));
  ASSERT_TRUE(ptr->IsDefined("Fn2"));
  ASSERT_FALSE(ptr->IsDefined("FnXXX"));

  Pool::Ptr   pool = Pool::Create("external mt pool", 1);
  Exec::Ptr   exec = ExecEngine::Create("external mt exec", pool);
  Future::Ptr fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- external mt test script"
		   "\n local future = lib.aliLua.future.Create()"
		   "\n local pool   = lib.aliLua.threading.CreatePool {"
		   "\n     name       = 'test pool',"
		   "\n     numThreads = 2,"
		   "\n }"
		   "\n local obj = lib.aliLua.externalObject.Create({"
		   "\n    mtName     = 'myJunkMT',"
		   "\n    engineName = 'myJunkEngine',"
		   "\n    future     = nil,"
		   "\n    threadPool = pool,"
		   "\n }, 'arg1', 'arg2')"
		   "\n assert(obj, 'did not retrieve the external mt')"
		   "\n");
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}

TEST(aliLuaExtExternalMT, scriptInterface) {
  Pool::Ptr   pool = Pool::Create("external mt pool", 1);
  Exec::Ptr   exec = ExecEngine::Create("external mt exec", pool);
  Future::Ptr fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- external mt test Lua test script interface"
		   "\n local arg = {"
		   "\n    mtName        = 'myExtMTJunk',"
		   "\n    script        = 'junkScript',"
		   "\n    createFn      = 'Create',"
		   "\n    onDestroyFn   = 'OnDestroy',"
		   "\n    functions     = {"
		   "\n      'Fn1',"
		   "\n      'Fn2',"
		   "\n    },"
		   "\n }"
		   "\n local mt = lib.aliLua.externalMT.Create(arg)"
		   "\n assert(mt, 'external MT was not created')"
		   "\n local info = mt:GetInfo()"
		   "\n function EQ(field)"
		   "\n    assert(info[field]==arg[field], 'bad ' .. field)"
		   "\n end"
		   "\n EQ('mtName')"
		   "\n EQ('script')"
		   "\n EQ('createFn')"
		   "\n EQ('onDestroyFn')"
		   "\n local fSet = { Fn1 = true, Fn2 = true, }"
		   "\n assert(#info.functions==2, 'unexpected number of functions')"
		   "\n for _,fnName in ipairs(info.functions) do"
		   "\n    assert(fSet[fnName], 'unrecognized function name')"
		   "\n    fSet[fnName] = nil"
		   "\n end"
		   "\n assert(next(fSet)==nil, 'not all functions returned')"
		   );
  TestUtil::Wait(exec, fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError()) << fPtr->GetError();
}
