#include <app1.hpp>
#include <aliLuaCore.hpp>
#include <lua.hpp>
#include <unistd.h>

namespace {

  int Fn1(lua_State *) {
    INFO("in Fn1");
    return 0;
  }
  int Fn2(lua_State *) {
    INFO("in Fn2");
    return 0;
  }
  int Sleep(lua_State *L) {
    int delay = lua_tointeger(L,1);
    INFO("Sleeping " << delay);
    sleep(delay);
    return 0;
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("app1 functions");
    fnMap->Add("Fn1",   Fn1);
    fnMap->Add("Fn2",   Fn2);
    fnMap->Add("Sleep", Sleep);
    aliLuaCore::Module::Register("load app1 functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.app1",  fnMap);
				 });
  }
  void Fini() {
  }

}


namespace app1 {
  
  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("app1", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }

}
