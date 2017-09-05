#include <aliLuaExt2_action.hpp>
#include <aliSystem.hpp>
#include <set>

namespace {

  using OBJ = aliLuaExt2::Action::OBJ;
  aliSystem::Stats::Ptr actionTimeoutStats;

  void ExtractReferences(lua_State                  *L,
			 int                         tableIndex,
			 const std::string          &tableField,
			 aliLuaExt2::Action::DepVec &depVec) {
    aliLuaCore::StackGuard g(L,3);
    tableIndex = lua_absindex(L,tableIndex);
    if (lua_istable(L,tableIndex)) {
      lua_getfield(L,tableIndex, tableField.c_str());
      int depTable = lua_gettop(L);
      if (lua_istable(L,depTable)) {
	lua_pushnil(L);
	while (lua_next(L,depTable)) {
	  int type = lua_type(L,-1);
	  if (type==LUA_TUSERDATA) {
	    if (aliLuaCore::Future::OBJ::Is(L,-1)) {
	      aliLuaCore::Future::OBJ::TPtr ref = aliLuaCore::Future::OBJ::Get(L,-1,false);
	      depVec.push_back(ref);
	    } else if (OBJ::Is(L,-1)) {
	      aliLuaExt2::Action::Ptr act = OBJ::Get(L, -1, false);
	      depVec.push_back(act->GetResult());
	    } else {
	      THROW("Unrecognized dependency - value is not a future or action");
	    }
	  } else {
	    THROW("Unrecognized dependency - value is not a user data value");
	  }
	  lua_pop(L,1);
	}
      }
    }
  }
  
  int Create(lua_State *L) {
    aliLuaExt2::Action::Ptr     aPtr;
    std::string                 name;
    aliLuaCore::CallTarget::Ptr target;
    aliLuaCore::Exec::Ptr       exec;
    aliLuaCore::MakeFn          args;
    double                      timeoutDelay; // seconds from current time
    aliLuaExt2::Action::DepVec  depVec;
    aliLuaCore::Table::GetString              (L, 1, "name",         name,   false);
    aliLuaCore::CallTarget::OBJ::GetTableValue(L, 1, "target",       target, false);
    aliLuaCore::Exec::OBJ::GetTableValue      (L, 1, "exec",         exec,   false);
    aliLuaCore::Table::GetDouble              (L, 1, "timeoutDelay", timeoutDelay, false);
    args = aliLuaCore::Values::GetMakeFnRemaining(L, 2);
    ExtractReferences(L, 1, "dependencies", depVec);
    aliSystem::Time::TP utcTimeout = aliSystem::Time::Now();
    utcTimeout += aliSystem::Time::FromSeconds(timeoutDelay);
    aPtr = aliLuaExt2::Action::Create(name, target, exec, args, utcTimeout, depVec);
    return OBJ::Make(L,aPtr);
  }
  int GetInfo(lua_State *L) {
    aliLuaExt2::Action::Ptr   ptr = OBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil rtn;
    aliLuaCore::MakeTableUtil depList;
    int                   index = 0;
    ptr->ForEachDependency([&](const aliLuaCore::Future::Ptr &fPtr) {
	depList.SetMakeFnForIndex(++index, aliLuaCore::Future::OBJ::GetMakeFn(fPtr));
      });
    rtn.SetString ("name",          ptr->GetName());
    rtn.SetMakeFn ("dependencies",  depList.GetMakeFn());
    rtn.SetBoolean("isReady",       ptr->IsReady());
    rtn.SetBoolean("hasRun",        ptr->HasRun());
    rtn.SetMakeFn ("result",        aliLuaCore::Future::OBJ::GetMakeFn(ptr->GetResult()));
    rtn.SetMakeFn ("target",        aliLuaCore::CallTarget::OBJ::GetMakeFn(ptr->GetTarget()));
    rtn.SetMakeFn ("exec",          aliLuaCore::Exec::OBJ::GetMakeFn(ptr->GetExec()));
    return rtn.GetMakeFn()(L);
  }
  int SetTimeout(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    ptr->SetTimeout();
    return 0;
  }
  int SetError(lua_State *L) {
    OBJ::TPtr   ptr = OBJ::Get(L,1,false);
    std::string err = aliLuaCore::Values::GetString(L,2);
    ptr->SetError(err);
    return 0;
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("Lua Action functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("Lua Action MT");
    mtMap->Add("GetInfo",    GetInfo);
    mtMap->Add("SetTimeout", SetTimeout);
    mtMap->Add("SetError",   SetError);
    OBJ::Init("luaAction", mtMap, true);
    aliLuaCore::Module::Register("load aliLuaExt2::Action functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.action",  fnMap);
				   OBJ::Register(ePtr);
				 });
    actionTimeoutStats = aliSystem::Stats::Create("action timeout");
  }
  void Fini() {
    OBJ::Fini();
  }


  void OnTimeout(const aliLuaExt2::Action::WPtr &wPtr) {
    aliLuaExt2::Action::Ptr ptr = wPtr.lock();
    if (ptr) {
      ptr->SetTimeout();
    }
  }
}

namespace aliLuaExt2 {

  void Action::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt2::Action", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }

  Action::Ptr Action::Create(const std::string                 &name,
			     const aliLuaCore::CallTarget::Ptr &target,
			     const aliLuaCore::Exec::Ptr       &exec,
			     const aliLuaCore::MakeFn          &args,
			     const aliSystem::Time::TP         &utcTimeout, 
			     const DepVec                      &dependencies) {
    Ptr rtn(new Action);
    rtn->name         = name;
    rtn->target       = target;
    rtn->exec         = exec;
    rtn->args         = args;
    rtn->utcTimeout   = utcTimeout;
    rtn->dependencies = dependencies;
    rtn->result       = aliLuaCore::Future::Create();
    RegisterDependencies(rtn);
    rtn->Trigger();
    WPtr wPtr = rtn;
    aliSystem::Threading::Work::Ptr timeoutWork =
      aliSystem::Threading::Work::Create(actionTimeoutStats,
					 [=] (bool &) {
					   OnTimeout(wPtr);
					 });
    aliSystem::Threading::Scheduler::Schedule(utcTimeout, timeoutWork);
    return rtn;
  }
  const std::string                 &Action::GetName  () const { return name;   }
  const aliLuaCore::CallTarget::Ptr &Action::GetTarget() const { return target; }
  const aliLuaCore::Exec::Ptr       &Action::GetExec  () const { return exec;   }
  const aliLuaCore::Future::Ptr     &Action::GetResult() const { return result; }
  Action::~Action() {}
  void Action::ForEachDependency(const DepFn &fn) {
    std::lock_guard<std::mutex> g(lock);
    for (DepVec::iterator it=dependencies.begin(); it!=dependencies.end(); ++it) {
      const aliLuaCore::Future::Ptr &dep = *it;
      fn(dep);
    }
  }
  bool Action::IsReady() const {
    for (DepVec::const_iterator it=dependencies.begin(); it!=dependencies.end(); ++it) {
      aliLuaCore::Future::Ptr fPtr = *it;
      if (!fPtr->IsSet()) {
	return false;
      }
    }
    return true;
  }
  bool Action::HasRun() const {
    return hasRun;
  }
  void Action::SetTimeout() {
    std::lock_guard<std::mutex> g(lock);
    if (!hasRun) {
      hasRun = true;
      result->SetTimeout();
    }
  }
  void Action::SetError(const std::string &err) {
    std::lock_guard<std::mutex> g(lock);
    if (!hasRun) {
      hasRun = true;
      result->SetError(err);
    }
  }
  void Action::Trigger() {
    std::lock_guard<std::mutex> g(lock);
    if (!hasRun && IsReady()) {
      hasRun = true; // prevent double queuing
      target->Run(exec, result, args);
    }
  }
  std::ostream &operator<<(std::ostream &out, const Action &o) {
    out << "Action(" << o.GetName() << ")";
    return out;
  }
  Action::Action()
    : hasRun(false) {
  }
  void Action::RegisterDependencies(const Ptr &act) {
    if (!act->HasRun()) {
      WPtr wAct = act;
      for (DepVec::iterator it=act->dependencies.begin();
	   it!=act->dependencies.end();
	   ++it) {
	const aliLuaCore::Future::Ptr  &fPtr  = *it;
	aliSystem::Listeners<aliLuaCore::Future*>::Register(fPtr->OnSet(), "actionListener",
							    [=](aliLuaCore::Future *future) {
							      Action::Trigger(wAct, future);
							    }, false);
      }
    }
  }
  void Action::Trigger(const WPtr &wAct, aliLuaCore::Future *dep) {
    Ptr act = wAct.lock();
    if (act) {
      if (dep) {
	if (dep->IsError()) {
	  act->SetError("dependency error");
	} else {
	  act->Trigger();
	}
      }
    }
  }
    
}
