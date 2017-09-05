#include <aliLuaCore_callTarget.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_future.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_table.hpp>
#include <aliLuaCore_util.hpp>

namespace {
  using OBJ       = aliLuaCore::CallTarget::OBJ;
  using TargetMap = std::map<std::string, aliLuaCore::CallTarget::CreateFn>;
  std::mutex lock;
  TargetMap  targetMap;

  int Create(lua_State *L) {
    std::string targetType;
    aliLuaCore::CallTarget::CreateFn createFn;
    aliLuaCore::Table::GetString(L,1,"targetType", targetType, false);
    if (!targetType.empty()) {
      std::lock_guard<std::mutex> g(lock);
      TargetMap::iterator it = targetMap.find(targetType);
      if (it!=targetMap.end()) {
	createFn = it->second;
      }
    }
    THROW_IF(!createFn, "Unrecognized target type: " << targetType);
    return OBJ::Make(L,createFn(L));
  }
  int GetInfo(lua_State *L) {
    OBJ::TPtr          tPtr = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn info = tPtr->GetInfo();
    return info(L);
  }
  int Run(lua_State *L) {
    OBJ::TPtr               tPtr         = OBJ::Get(L,1,false);
    const int               optionsTable = 2;
    aliLuaCore::MakeFn      args         = aliLuaCore::Values::GetMakeFnRemaining(L,3);
    aliLuaCore::Future::Ptr fPtr;
    aliLuaCore::Exec::Ptr   ePtr;
    if (!lua_isnil(L,optionsTable)) {
      THROW_IF(!lua_istable(L,optionsTable), "Options table argument is not a table");
      aliLuaCore::Future::OBJ::GetTableValue(L, optionsTable, "future", fPtr, true);
      aliLuaCore::Exec::OBJ  ::GetTableValue(L, optionsTable, "exec",   ePtr, false);
    }
    tPtr->Run(ePtr, fPtr, args);
    return 0;
  }
  
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("call target functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("call target MT");
    mtMap->Add("GetInfo",  GetInfo);
    mtMap->Add("Run",      Run);
    OBJ::Init("luaCallTarget", mtMap, true);
    aliLuaCore::Module::Register("load aliLuaCore::Call functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr,
							       "lib.aliLua.callTarget",
							       fnMap);
				   OBJ::Register(ePtr);
				 });
  }
  void Fini() {
    OBJ::Fini();
  }

}

namespace aliLuaCore {

  void CallTarget::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::CallTarget", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::Module");
  }
  
  CallTarget::CallTarget() {
  }
  CallTarget::~CallTarget() {}
  void CallTarget::Run(const ExecPtr &ePtr, const MakeFn &args) {
    Future::Ptr fPtr;
    this->Run(ePtr, fPtr, args);
  }
  void CallTarget::Register(const std::string &targetType, const CreateFn &createFn) {
    std::lock_guard<std::mutex> g(lock);
    TargetMap::iterator it = targetMap.find(targetType);
    THROW_IF(it!=targetMap.end(), "Attempt to re-register " << targetType);
    targetMap[targetType] = createFn;
  }
  void CallTarget::Unregister(const std::string &targetType) {
    std::lock_guard<std::mutex> g(lock);
    WARN_IF(!targetMap.erase(targetType), "Unregistering an unrecognized target type " << targetType);
  }
  
}
