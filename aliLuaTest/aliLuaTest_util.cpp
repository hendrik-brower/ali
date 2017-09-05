#include <aliLuaTest_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <unistd.h>

namespace {
  namespace locals {
    
    int Sleep(lua_State *L) {
      double delayInSeconds = lua_tonumber(L,1);
      usleep(delayInSeconds*1000*1000);
      return 0;
    }
    void Init() {
      aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("test util funtions");
      fnMap->Add("Sleep",   locals::Sleep);
      aliLuaCore::Module::Register("load threading test functions",
			       [=](const aliLuaCore::Exec::Ptr &exec) {
				 aliLuaCore::Util::LoadFnMap(exec,
							     "lib.aliLuaTest.testUtil",
							     fnMap);
			       });
    }
    void Fini() {
    }
    
  }
}
namespace aliLuaTest {

  Util::LPtr Util::GetL() {
    LPtr rtn(luaL_newstate(), lua_close);
    return rtn;
  }
  void Util::Wait(const aliLuaCore::Exec::Ptr &exec, const aliLuaCore::Future::Ptr &fPtr) {
    aliSystem::Threading::Semaphore::Ptr sem(new aliSystem::Threading::Semaphore);
    for (int i=0; i<100; ++i) {
      if (fPtr && fPtr->IsSet()) {
	break;
      }
      usleep(1000);
      exec->Run([=](lua_State *) {
	  sem->Post();
	  return 0;
	});
      sem->Wait();
    }
  }

  bool Util::DidThrow(const std::function<void()> &fn) {
    bool didThrow = false;
    try {
      fn();
    } catch (std::exception &e) {
      didThrow = true;
    }
    return didThrow;
  }

  void Util::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaTest_util", locals::Init, locals::Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }

}
