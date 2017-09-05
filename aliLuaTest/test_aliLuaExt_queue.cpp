#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>

namespace {
  using ExecEngine = aliLuaExt::ExecEngine;
  using Future     = aliLuaCore::Future;
  using Listeners  = aliLuaExt::Queue::Listeners;
  using Queue      = aliLuaExt::Queue;
  using Util       = aliLuaCore::Util;
  using IPtr       = aliLuaTest::Util::IPtr;
  using IVec       = aliLuaTest::Util::IVec;
  using IVecPtr    = aliLuaTest::Util::IVecPtr;
  using TestUtil   = aliLuaTest::Util;
  using Pool       = aliSystem::Threading::Pool;
}

TEST(aliLuaExtQueue, cppInterface) {
  lua_State        *L = nullptr;
  IPtr              onFirst(new int(0));
  IPtr              onInsert(new int(0));
  IVecPtr           ivp(new IVec);
  const std::string name = "test queue";
  Queue::Ptr        ptr  = Queue::Create(name);
  Listeners::Register(ptr->OnFirst(), "abc", [=](const Queue::WPtr&) {
      ++(*onFirst);
    }, false);
  Listeners::Register(ptr->OnInsert(), "123", [=](const Queue::WPtr&) {
      ++(*onInsert);
    }, false);
  ASSERT_NE(ptr.get(), nullptr);
  ASSERT_EQ(ptr.get(), ptr->GetPtr().get());
  ASSERT_STREQ(name.c_str(), ptr->Name().c_str());
  ASSERT_EQ(ptr->Size(),0u);
  ASSERT_TRUE(ptr->Empty());
  ASSERT_EQ(*onFirst,  0);
  ASSERT_EQ(*onInsert, 0);
  ptr->Append([=](lua_State *) {
      ivp->push_back(2);
      return 1;
    });
  ASSERT_EQ(ptr->Size(), 1u);
  ASSERT_FALSE(ptr->Empty());
  ASSERT_EQ(*onFirst,  1);
  ASSERT_EQ(*onInsert, 1);
  ptr->Insert([=](lua_State *) {
      ivp->push_back(1);
      return 0;
    });
  ASSERT_EQ(ptr->Size(), 2u);
  ASSERT_EQ(*onFirst,  1);
  ASSERT_EQ(*onInsert, 2);
  ptr->Append([=](lua_State *) {
      ivp->push_back(3);
      return 2;
    });
  ASSERT_EQ(ptr->Size(), 3u);
  ASSERT_EQ(*onFirst,  1);
  ASSERT_EQ(*onInsert, 3);

  for (int i=0;i<3;++i) {
    int rc = ptr->PushNext(L);
    ASSERT_EQ(rc,i);
  }
  ASSERT_TRUE(ptr->Empty());
  ASSERT_EQ(ivp->operator[](0), 1);
  ASSERT_EQ(ivp->operator[](1), 2);
  ASSERT_EQ(ivp->operator[](2), 3);
  ptr->Insert([=](lua_State *) {
      ivp->push_back(3);
      return 4;
    });
  ASSERT_EQ(ptr->Size(), 1u);
  ASSERT_EQ(*onFirst,  2);
  ASSERT_EQ(*onInsert, 4);
}
    
TEST(aliLuaExtQueue, luaInterface) {
  const std::string name = "queue test";
  Pool::Ptr         pool = Pool::Create(name, 1);
  ExecEngine::Ptr   exec = ExecEngine::Create(name, pool);
  Future::Ptr       fPtr = Future::Create();
  Util::LoadString(exec, fPtr,
		   "-- Lua queue testing"
		   "\n onFirstCount  = 0"
		   "\n onInsertCount = 0"
		   "\n queueName     = 'testQueue'"
		   "\n exec          = lib.aliLua.exec.GetEngine()"
		   "\n function OnFirst(queue)"
		   "\n   if queue then"
		   "\n      onFirstCount = onFirstCount + 1"
		   "\n   end"
		   "\n end"
		   "\n function OnInsert(queue)"
		   "\n   if queue then"
		   "\n      onInsertCount = onInsertCount + 1"
		   "\n   end"
		   "\n end"
		   "\n queue = lib.aliLua.queue.Create {"
		   "\n    name = queueName"
		   "\n }"
		   "\n onFirstTarget = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'functionTarget',"
		   "\n    fnName     = 'OnFirst',"
		   "\n }"
		   "\n onInsertTarget = lib.aliLua.callTarget.Create {"
		   "\n    targetType = 'functionTarget',"
		   "\n    fnName     = 'OnInsert',"
		   "\n }"
		   "\n onFirst = lib.aliLua.queue.CreateListener {"
		   "\n     target = onFirstTarget,"
		   "\n     exec   = exec,"
		   "\n }"
		   "\n onInsert = lib.aliLua.queue.CreateListener {"
		   "\n     target = onInsertTarget,"
		   "\n     exec   = exec,"
		   "\n }"
		   "\n queue:OnFirst {"
		   "\n     listener = onFirst,"
		   "\n     useWeak  = true,"
		   "\n }"
		   "\n queue:OnInsert {"
		   "\n     listener = onInsert,"
		   "\n     useWeak  = true,"
		   "\n }"
		   "\n -- checks"
		   "\n assert(queue:Name()==queueName, 'invalid queue:Name()')"
		   "\n assert(queue:Empty(), 'queue:Empty()')"
		   "\n assert(queue:Size()==0, 'queue:Size()~=0')"
		   "\n queue:Append {val=1}"
		   "\n assert(not queue:Empty(), 'not queue:Empty()')"
		   "\n assert(queue:Size()==1, 'queue:Size()~=1')"
		   "\n queue:Insert {val=0}"
		   "\n assert(queue:Size()==2, 'queue:Size()~=2')"
		   "\n local item = queue:Next()"
		   "\n assert(queue:Size()==1, 'queue:Size()~=1')"
		   "\n assert(item.val==0, 'item.val ~= 0')"
		   "\n item = queue:Next()"
		   "\n assert(queue:Size()==0, 'queue:Size()~=0')"
		   "\n assert(item.val==1, 'item.val ~= 1')"
		   "\n");
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
  fPtr = Future::Create();
  Util::LoadString(exec,fPtr,
		   "-- Lua queue testing, part 2"
		   "\n -- release objects"
		   "\n queue         :Release()"
		   "\n onFirst       :Release()"
		   "\n onInsert      :Release()"
		   "\n onFirstTarget :Release()"
		   "\n onInsertTarget:Release()"
		   "\n exec          :Release()"
		   "\n "
		   "\n -- verify counts"
		   "\n assert(onFirstCount  == 1, 'onFirstCount  ~= 1')"
		   "\n assert(onInsertCount == 2, 'onInsertCount ~= 2')"
		   "\n");
  TestUtil::Wait(exec,fPtr);
  ASSERT_TRUE(fPtr->IsSet());
  ASSERT_FALSE(fPtr->IsError());
}
