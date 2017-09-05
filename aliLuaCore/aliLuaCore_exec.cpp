#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_future.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_stats.hpp>
#include <aliLuaCore_values.hpp>

namespace {

  // ****************************************************************************************
  // Component
  using OBJ = aliLuaCore::Exec::OBJ;

  int GetInfo(lua_State *L) {
    OBJ::TPtr          ptr  = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn info = ptr->GetInfo();
    return info(L);
  }
  int SetStats(lua_State *L) {
    OBJ::TPtr                    ptr  = OBJ::Get(L,1,false);
    aliLuaCore::Stats::OBJ::TPtr sPtr = aliLuaCore::Stats::OBJ::Get(L,2,true);
    ptr->SetStats(sPtr);
    return 0;
  }
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("exec functions");
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("exec MT");
    mtMap->Add("GetInfo",  GetInfo);
    mtMap->Add("SetStats", SetStats);
    OBJ::Init("luaExec", mtMap, true);
    aliLuaCore::Module::Register("load aliLuaCore::Exec functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.exec",  fnMap);
				   OBJ::Register(ePtr);
				 });
  }
  void Fini() {
    OBJ::Fini();
  }

}

namespace aliLuaCore {

  void Exec::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::Exec", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::Module");
  }
  const std::string               &Exec::GetExecType() const { return execType; }
  const std::string               &Exec::Name       () const { return name; }
  const aliSystem::Stats::Ptr     &Exec::Stats      () const { return statsPtr; }
  const aliSystem::Registry::Ptr  &Exec::Registry   () const { return registry; }
  MakeFn Exec::GetInfo() {
    return info.GetMakeFn();
  }
  void Exec::SetInfo(const std::string &key, const MakeFn &makeFn) {
    info.SetMakeFn(key, makeFn);
  }
  void Exec::SetStats(const aliSystem::Stats::Ptr &sPtr) {
    statsPtr = sPtr;
  }
  void Exec::Run(const LuaFn &luaFn) {
    Future::Ptr future;
    Run(future, luaFn);
  }
  void Exec::Run(const FuturePtr   &future,
		 const LuaFn       &luaFn) {
    InternalRun(future, luaFn);
  }
  Exec::~Exec() {}

  std::ostream &operator<<(std::ostream &out, const Exec &o) {
    out << "Exec(" << o.name << ")";
    return out;
  }
  Exec::Exec(const std::string &execType_, const std::string &name_)
    : execType(execType_),
      name(name_),
      statsPtr(new aliSystem::Stats(name_)),
      registry(aliSystem::Registry::Create("objstore")) {
    info.SetMakeFn("execInfo", [=](lua_State *L) {
	MakeTableUtil mtu;
	mtu.SetString("execType", this->GetExecType());
	mtu.SetString("name",     this->Name());
	mtu.SetMakeFn("stats",    Stats::OBJ::GetMakeFn(this->Stats()));
	mtu.SetMakeFn("exec",     OBJ::GetMakeWeakFn(this->GetExec()));
	return mtu.Make(L);
      });
  }

}


