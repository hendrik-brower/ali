#include <aliLuaExt_functionTarget.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {
  namespace {
    const std::string targetType = "functionTarget";

    aliLuaCore::CallTarget::Ptr Create(lua_State *L) {
      std::string         fnName;
      aliLuaCore::Table::GetString(L,1,"fnName", fnName, false);
      THROW_IF(fnName.empty(), "fnName is an empty string");
      FunctionTarget::Ptr rtn(new FunctionTarget(fnName));
      return rtn;
    }
    void Init() {
      aliLuaCore::CallTarget::Register(targetType, Create);
    }
    void Fini() {
      aliLuaCore::CallTarget::Unregister(targetType);
    }
  }

  void FunctionTarget::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::FunctionTarget", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }
  
  FunctionTarget::FunctionTarget(const std::string &fnName_)
    : fnName(fnName_) {
  }
  void FunctionTarget::Run(const aliLuaCore::Exec::Ptr   &ePtr,
			   const aliLuaCore::Future::Ptr &fPtr,
			   const aliLuaCore::MakeFn      &args) {
    THROW_IF(!ePtr, "Attempt to run a function call target with an uninitialized exec pointer");
    aliLuaCore::Util::RunFn(ePtr, fPtr, fnName, args);
  }
  const std::string &FunctionTarget::FnName() const { return fnName; }

  aliLuaCore::MakeFn FunctionTarget::GetInfo() {
    aliLuaCore::MakeTableUtil tbl;
    tbl.SetString("fnName", fnName);
    return tbl.GetMakeFn();
  }    

  
}
