#include <aliLuaCore_stats.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_functionMap.hpp>
#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_module.hpp>

namespace {

  using OBJ = aliLuaCore::Stats::OBJ;

  int Create(lua_State *L) {
    const std::string name = aliLuaCore::Values::GetString(L,1);
    OBJ::TPtr         ptr(new aliSystem::Stats(name));
    return OBJ::Make(L,ptr);
  }
  int GetInfo(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn info = aliLuaCore::Stats::Info(ptr);
    return info(L);
  }
  int Inc(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    double    tm  = lua_tonumber(L,2); // seconds
    ptr->Inc(aliSystem::Time::FromSeconds(tm));
    return 0;
  }
  
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("stats functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("stats MT");
    mtMap->Add("GetInfo", GetInfo);
    mtMap->Add("Inc",     Inc);
    OBJ::Init("stats", mtMap, true);
    aliLuaCore::Module::Register("load stats functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.stats",  fnMap);
			       OBJ::Register(ePtr);
			     });
  }
  void Fini() {
    OBJ::Fini();
  }
  
}

namespace aliLuaCore {

  void Stats::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::Stats", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::Module");
  }

  MakeFn Stats::Info(const aliSystem::Stats::Ptr &ptr) {
    MakeTableUtil mtu;
    THROW_IF(!ptr, "Uninitialized pointer");
    mtu.SetString("name",    ptr->Name());
    mtu.SetNumber("count",   (double)ptr->Count());
    mtu.SetNumber("runTime", aliSystem::Time::ToSeconds(ptr->RunTime()));
    return mtu.GetMakeFn();
  }

}
