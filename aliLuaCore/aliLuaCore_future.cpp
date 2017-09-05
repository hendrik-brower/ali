#include <aliLuaCore_future.hpp>
#include <aliLuaCore_callTarget.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_table.hpp>
#include <aliLuaCore_types.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_values.hpp>
#include <aliSystem.hpp>

namespace {
  using OBJ  = aliLuaCore::Future::OBJ;
  using LOBJ = aliLuaCore::Future::LOBJ;
  
  int Create(lua_State *L) {
    return OBJ::Make(L,aliLuaCore::Future::Create());
  }
  int IsSet(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    lua_checkstack(L,1);
    lua_pushboolean(L,ptr->IsSet());
    return 1;
  }
  int GetValue(lua_State *L) {
    OBJ::TPtr ptr  = OBJ::Get(L,1,false);
    // FIXME: bool      wait = lua_toboolean(L,2);
    const aliLuaCore::MakeFn &value = ptr->GetValue();
    THROW_IF(!value, "Value has not been assigned");
    return value(L);
  }
  int SetValue(lua_State *L) {
    OBJ::TPtr          ptr   = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn value = aliLuaCore::Values::GetMakeFnRemaining(L,2);
    ptr->SetValue(value);
    return 0;
  }
  int OnSet(lua_State *L) {
    aliLuaCore::CallTarget::Ptr                   target;
    aliLuaCore::Exec::Ptr                         exec;
    aliLuaCore::MakeFn                            args;
    aliSystem::Listener<aliLuaCore::Future*>::Ptr lPtr;
    OBJ::TPtr                                     ptr = OBJ::Get(L,1,false);
    aliLuaCore::CallTarget::OBJ::GetTableValue(L, 2, "target", target, false);
    aliLuaCore::Exec      ::OBJ::GetTableValue(L, 2, "exec",   exec,   false);
    args = aliLuaCore::Values::GetMakeFnRemaining(L,3);
    lPtr = aliSystem::Listeners<aliLuaCore::Future*>::Register(ptr->OnSet(),
							       "luaListener",
							       [=](aliLuaCore::Future *) {
								 target->Run(exec, args);
							       }, false);
    return aliLuaCore::Future::LOBJ::Make(L,lPtr);
  }
  void Init() {
    if (true) {
      // init future
      aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("future functions");
      fnMap->Add("Create",    Create);
      aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("future MT");
      mtMap->Add("IsSet",    IsSet);
      mtMap->Add("GetValue", GetValue);
      mtMap->Add("SetValue", SetValue);
      mtMap->Add("OnSet",    OnSet);
      OBJ::Init("luaFuture", mtMap, true);
      aliLuaCore::Module::Register("load aliLuaCore::Future functions",
			       [=](const aliLuaCore::Exec::Ptr &ePtr) {
				 aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.future",  fnMap);
				 OBJ::Register(ePtr);
			       });
    }
    if (true) {
      // init Listener<Future>
      aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("future listener MT");
      LOBJ::Init("luaListener", mtMap, true);
      aliLuaCore::Module::Register("load aliLuaCore::Listener functions",
			       [=](const aliLuaCore::Exec::Ptr &ePtr) {
				 LOBJ::Register(ePtr);
			       });
    }
  }
  void Fini() {
    OBJ::Fini();
    LOBJ::Fini();
  }

}

namespace aliLuaCore {
  
  void Future::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::Future", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::Module");
  }

  Future::Ptr Future::Create() {
    Ptr rtn(new Future);
    rtn->onSet = aliSystem::Listeners<Future*>::Create("OnSet");
    return rtn;
  }
  Future::Ptr Future::Create(const MakeFn &value) {
    Ptr rtn = Create();
    rtn->SetValue(value);
    return rtn;
  }
  Future::~Future() {}
  void Future::SetValue(const MakeFn &value_) {
    MakeFnVec mVec;
    mVec.push_back(Values::MakeTrue);
    mVec.push_back(value_);
    SetValueInternal(Values::GetMakeArrayFn(mVec), false, "");
  }
  void Future::SetTimeout() {
    // Could add additional values to the timeout like timeout time.
    // If that is done, convert 'timeout' from static to non-static
    // and update the makeFn in an appropriate manner.
    static MakeFn timeout;
    if (!timeout) {
      static std::mutex lock;
      std::lock_guard<std::mutex> g(lock);
      if (!timeout) {
	MakeFnVec mVec;
	mVec.push_back(Values::MakeFalse);
	mVec.push_back(Values::GetMakeStringFn("timeout"));
	timeout = Values::GetMakeArrayFn(mVec);
      }
    }
    SetValueInternal(timeout, true, "timeout");
  }
  void Future::SetError(const std::string &err) {
    MakeFnVec mVec;
    mVec.push_back(Values::MakeFalse);
    mVec.push_back(Values::GetMakeStringFn(err));
    SetValueInternal(Values::GetMakeArrayFn(mVec), true, err);
  }

  bool Future::IsSet() const { return !!value; }
  bool Future::IsError() const { return isError; }
  const std::string &Future::GetError() const { return error; }
  const MakeFn &Future::GetValue() const { return value; }
  const aliSystem::Listeners<Future*>::Ptr &Future::OnSet() { return onSet; }
  Future::Future()
    : isError(false) {
  }
  void Future::SetValueInternal(const MakeFn     &value_,
				bool              isError_,
				const std::string &error_) {
    if (!onSet->Notify(this, [=]()->bool {
	  bool wasNotSet = !this->value;
	  if (!this->value) {
	    this->value   = value_;
	    this->isError = isError_;
	    this->error   = error_;
	  }
	  return wasNotSet;
	})) {
      THROW("Value has already been set");
    }
  }

}

