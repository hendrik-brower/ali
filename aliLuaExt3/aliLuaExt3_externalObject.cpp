#include <aliLuaExt3_externalObject.hpp>
#include <aliLuaExt3_externalMT.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>

namespace {
  using OBJ = aliLuaExt3::ExternalObject::OBJ;

  int Create(lua_State *L) {
    std::string                     mtName;
    std::string                     engineName;
    aliLuaCore::MakeFn              args;        // object initialization argument
    aliLuaCore::MakeFn              queueObj;    // needs to support a 'Append' metamethod.
    aliLuaCore::Future::Ptr         loadResult;
    aliLuaCore::Future::Ptr         createResult;
    aliSystem::Threading::Pool::Ptr threadPool;
    lua_checkstack(L,1);
    THROW_IF(!lua_istable(L, 1), "Expected a table");
    aliLuaCore::Table::GetString(L, 1, "mtName",     mtName, false);
    aliLuaCore::Table::GetString(L, 1, "engineName", engineName, false);
    aliLuaCore::Future::OBJ::GetTableValue (L, 1, "loadResult", loadResult, true);
    aliLuaCore::Future::OBJ::GetTableValue (L, 1, "createResult", createResult, true);
    args = aliLuaCore::Values::GetMakeFnRemaining(L, 2);
    aliLuaExt::Threading::PoolOBJ::GetTableValue(L, 1, "threadPool", threadPool, true);
    aliLuaExt3::ExternalMT::Ptr emtPtr = aliLuaExt3::ExternalMT::Get(mtName);
    THROW_IF(!emtPtr, "Unrecognized name");
    return OBJ::Make(L, aliLuaExt3::ExternalObject::Create(emtPtr,
							   engineName,
							   threadPool,
							   loadResult,
							   createResult,
							   args));
  }

  int GetInfo(lua_State *L) {
    OBJ::TPtr eoPtr = OBJ::Get(L,1,false);
    THROW_IF(!eoPtr, "Expecting an external object");
    aliLuaCore::MakeTableUtil mtu;
    mtu.SetMakeFn("exec", aliLuaCore::Exec::OBJ::GetMakeWeakFn(eoPtr->GetExec()));
    mtu.SetMakeFn("mt",   eoPtr->EMTPtr()->GetInfo());
    return mtu.Make(L);
  }
  int Call(lua_State *L) {
    std::string                 fnName;
    aliLuaCore::Future::Ptr     callResult;
    aliLuaCore::MakeFn          args;
    OBJ::TPtr                   eoPtr  = OBJ::Get(L,1,false);
    aliLuaExt3::ExternalMT::Ptr emtPtr = eoPtr->EMTPtr();
    aliLuaCore::Table::GetString          (L, 2, "fnName",     fnName,     false);
    aliLuaCore::Future::OBJ::GetTableValue(L, 2, "callResult", callResult, true);
    args = aliLuaCore::Values::GetMakeFnRemaining(L, 3);
    THROW_IF(!emtPtr->IsDefined(fnName), "Attempt to call an"
	     " undefined function " << fnName << " for the"
	     " object " << emtPtr->MTName());
    eoPtr->RunFunction(fnName, callResult, args);
    if (callResult) {
      return aliLuaCore::Future::OBJ::Make(L,callResult);
    }
    return 0;
  }
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("external object functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("external object MT");
    mtMap->Add("GetInfo", GetInfo);
    mtMap->Add("Call",    Call);
    OBJ::Init("aliExternalObject",  mtMap,  true);
    aliLuaCore::Module::Register("load aliLuaExt::ExternalMT functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.externalObject",  fnMap);
				   OBJ::Register(ePtr);
				 });
  }
  void Fini() {
    OBJ::Fini();
  }

}

namespace aliLuaExt3 {

  void ExternalObject::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt3::ExternalObject", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }
  ExternalObject::Ptr ExternalObject::Create(const ExternalMT::Ptr                 &emtPtr,
					     const std::string                     &engineName,
					     const aliSystem::Threading::Pool::Ptr &threadPool,
					     const aliLuaCore::Future::Ptr         &loadResult,
					     const aliLuaCore::Future::Ptr         &createResult,
					     const aliLuaCore::MakeFn              &args) {
    THROW_IF(!emtPtr, "Attempt to create an external object with an uninitialized ExternalMT pointer");
    Ptr rtn(new ExternalObject);
    rtn->emtPtr   = emtPtr;
    rtn->ePtr     = aliLuaExt::ExecEngine::Create(engineName, threadPool);
    THROW_IF(!rtn->ePtr, "Null pointer");
    aliLuaCore::Util::LoadString(rtn->ePtr, loadResult,
				 emtPtr->Script());
    aliLuaCore::Util::RunFn(rtn->ePtr, createResult, emtPtr->CreateFn(), args);
    return rtn;
  }
  const aliLuaCore::Exec::Ptr &ExternalObject::GetExec() const { return ePtr;   }
  const ExternalMT::Ptr       &ExternalObject::EMTPtr () const { return emtPtr; }
  void ExternalObject::RunFunction(const std::string &fnName,
				   const aliLuaCore::Future::Ptr &future,
				   const aliLuaCore::MakeFn  &args) {
    aliLuaCore::Util::RunFn(ePtr, future, fnName, args);
  }

  ExternalObject::~ExternalObject() {
    if (!emtPtr->OnDestroyFn().empty()) {
      aliLuaCore::Util::RunFn(ePtr, emtPtr->OnDestroyFn());
    }
  }
  std::ostream &operator<<(std::ostream &out, const ExternalObject &o) {
    out << "ExternalObject("
	<< "name=" << o.GetExec()->Name()
	<< ")";
    return out;
  }
  ExternalObject::ExternalObject() {
  }
  
}
