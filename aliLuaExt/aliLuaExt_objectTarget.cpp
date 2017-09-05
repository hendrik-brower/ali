#include <aliLuaExt_objectTarget.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {
  namespace {
    const std::string targetType = "objectTarget";
    aliLuaCore::CallTarget::Ptr Create(lua_State *L) {
      aliLuaCore::MT::Ptr mtPtr;
      aliLuaCore::MakeFn  object;
      std::string         fnName;
      aliLuaCore::Table::GetString(L,1,"fnName", fnName, false);
      aliLuaCore::Table::GetMakeFn(L,1,"object", object, false);
      lua_getfield(L,1,"object");
      mtPtr = aliLuaCore::MT::GetMT(L,-1,false);
      ObjectTarget::Ptr   rtn(new ObjectTarget(fnName, mtPtr, object));
      return rtn;
    }
    void Init() {
      aliLuaCore::CallTarget::Register(targetType, Create);
    }
    void Fini() {
      aliLuaCore::CallTarget::Unregister(targetType);
    }
  }

  void ObjectTarget::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::ObjectTarget", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  
  ObjectTarget::ObjectTarget(const std::string         &fnName_,
			     const aliLuaCore::MT::Ptr &mtPtr_,
			     const aliLuaCore::MakeFn  &object_)
    : fnName(fnName_),
      mtPtr(mtPtr_),
      object(object_) {
    THROW_IF(!mtPtr,  "Attempt to create a metatable call with an uninitialized mt pointer");
    THROW_IF(!object, "Attempt to create a metatable Call with an uninitialized object");
  }
  void ObjectTarget::Run(const aliLuaCore::Exec::Ptr   &ePtr,
			 const aliLuaCore::Future::Ptr &fPtr,
			 const aliLuaCore::MakeFn      &args) {
    THROW_IF(!ePtr, "Attempt to run an object call target with an uninitialized exec pointer");
    aliLuaCore::Util::RunMT(ePtr, fPtr, object, fnName, args);
  }
  const std::string         &ObjectTarget::FnName   () const { return fnName; }
  const aliLuaCore::MT::Ptr &ObjectTarget::GetMT    () const { return mtPtr;  }
  const aliLuaCore::MakeFn  &ObjectTarget::GetObject() const { return object; }

  aliLuaCore::MakeFn ObjectTarget::GetInfo() {
    aliLuaCore::MakeTableUtil tbl;
    tbl.SetString("fnName", fnName);
    tbl.SetString("mtName", mtPtr->Name());
    return tbl.GetMakeFn();
  }

}
