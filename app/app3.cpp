#include <app3.hpp>
#include <aliLuaCore.hpp>
#include <lua.hpp>

namespace {

  int DupItem(lua_State *L) {
    aliLuaCore::MakeFn fn = aliLuaCore::Values::GetMakeFnForIndex(L,1);
    return fn(L);
  }

  int DupArgs(lua_State *L) {
    aliLuaCore::MakeFn fn = aliLuaCore::Values::GetMakeFnForAll(L);
    return fn(L);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("app3 functions");
    fnMap->Add("DupItem", DupItem);
    fnMap->Add("DupArgs", DupArgs);
    aliLuaCore::Module::Register("load app3 functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.app3",  fnMap);
				 });
  }
  void Fini() {
  }

}

namespace app3 {
  
  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("app3", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }

}
