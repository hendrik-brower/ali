#include <aliLuaExt_execEngine.hpp>
#include <aliLuaExt_execEngineWork.hpp>
#include <aliLuaExt_threading.hpp>
#include <lua.hpp>
#include <iostream>

namespace {
  static std::string engineExecType = "execEngine";
  using OBJ        = aliLuaExt::ExecEngine::OBJ;
  using QListener  = aliSystem::Threading::Queue::QListener;
  using QListeners = aliSystem::Threading::Queue::QListeners;
  int engineKey;

  int CreateEngine(lua_State *L) {
    std::string                     name;
    aliSystem::Threading::Pool::Ptr pool;
    aliLuaCore::Table::GetString (L,1, "name", name, false);
    aliLuaExt::Threading::PoolOBJ::GetTableValue(L,1,"threadPool", pool, false);
    return OBJ::Make(L,aliLuaExt::ExecEngine::Create(name, pool));
  }
  int GetEngine(lua_State *L) {
    OBJ::TPtr ptr = aliLuaExt::ExecEngine::GetEngine(L);
    if (ptr) {
      return OBJ::MakeWeak(L,ptr);
    }
    return 0;
  }
  int LoadString(lua_State *L) {
    aliLuaCore::Future::Ptr fPtr;
    std::string             str;
    OBJ::TPtr               ptr = OBJ::Get(L, 1, false);
    aliLuaCore::Table::GetString          (L, 2, "str",    str, false);
    aliLuaCore::Future::OBJ::GetTableValue(L, 2, "future", fPtr, true);
    aliLuaCore::Util::LoadString(ptr, fPtr, str);
    return 0;
  }
  int LoadFile(lua_State *L) {
    aliLuaCore::Future::Ptr fPtr;
    std::string             path;
    OBJ::TPtr               ptr = OBJ::Get(L, 1, false);
    aliLuaCore::Table::GetString          (L, 2, "path",   path, false);
    aliLuaCore::Future::OBJ::GetTableValue(L, 2, "future", fPtr, true);
    aliLuaCore::Util::LoadFile(ptr, fPtr, path);
    return 0;
  }
  
  int GetInfo(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn info = ptr->aliLuaCore::Exec::GetInfo();
    return info(L);
  }
  int SetStats(lua_State *L) {
    OBJ::TPtr                    ptr  = OBJ::Get(L,1,false);
    aliLuaCore::Stats::OBJ::TPtr sPtr = aliLuaCore::Stats::OBJ::Get(L,2,true);
    ptr->SetStats(sPtr);
    return 0;
  }

  // ****************************************************************************************
  // InitObj
  void Init() {
    engineKey = aliLuaCore::Util::GetRegistryKey();
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("exec engine functions");
    fnMap->Add("CreateEngine",    CreateEngine);
    fnMap->Add("GetEngine",       GetEngine);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("exec engine MT");
    mtMap->Add("LoadString", LoadString);
    mtMap->Add("LoadFile",   LoadFile);
    mtMap->Add("GetInfo",    GetInfo);
    mtMap->Add("SetStats",   SetStats);
    OBJ::Init("luaExecEngine", mtMap, true);
    aliLuaCore::MT::Ptr execMT = aliLuaCore::Exec::OBJ::GetMT();
    THROW_IF(!execMT, "Exec uninitialized");
    execMT->AddDerived(OBJ::GetMT());
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

namespace aliLuaExt {

  void ExecEngine::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::ExecEngine", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }
  // ****************************************************************************************
  // ExecEngine
  ExecEngine::Ptr ExecEngine::Create(const std::string &name,
				     const Pool::Ptr   &pool) {
    THROW_IF(!pool, "No thread pool supplied");
    Queue::Ptr qPtr = pool->AddQueue(name, 1, aliSystem::Stats::Create("execEngine queue"));
    Ptr  rtn(new ExecEngine(name));
    WPtr wRtn = rtn;
    rtn->THIS = rtn;
    rtn->queue  = qPtr;
    rtn->onIdle = Listeners::Create(name);
    rtn->queueListener = QListener::Create(name+"qListener", [=](const Queue::WPtr &) {
	Ptr ptr = wRtn.lock();
	if (ptr) {
	  ptr->onIdle->Notify(wRtn);
	}
      });
    QListeners::Register(qPtr->OnIdle(), rtn->queueListener, true);
    rtn->SetInfo("engineInfo", [=](lua_State *L) {
	aliLuaCore::MakeFn fn =  GetInfo(wRtn.lock());
	return fn(L);
      });
    luaL_openlibs(rtn->L);
    aliLuaCore::Module::InitEngine(rtn);
    return rtn;
  }
  ExecEngine::Ptr ExecEngine::GetEngine(lua_State *L) {
    Ptr        rtn;
    aliLuaCore::StackGuard g(L,1);
    aliLuaCore::Util::RegistryGet(L, engineKey);
    ExecEngine *p = (ExecEngine*)lua_touserdata(L,-1);
    if (p) {
      rtn = p->THIS.lock();
    }
    return rtn;
  }
  aliLuaCore::MakeFn ExecEngine::GetInfo(const Ptr &ptr) {
    THROW_IF(!ptr, "Uninitialized pointer");
    aliLuaCore::MakeTableUtil tbl;
    tbl.SetString ("name",       ptr->Name());
    tbl.SetNumber ("numPending", (int)(ptr->queue ? ptr->queue->NumPending() : 0));
    tbl.SetBoolean("isBusy",     ptr->IsBusy());
    tbl.SetString ("execType",   engineExecType);
    tbl.SetMakeFn ("engine",     ExecEngine::OBJ::GetMakeWeakFn(ptr));
    return tbl.GetMakeFn();
  }
  aliLuaCore::Exec::Ptr ExecEngine::GetExec() const {
    return THIS.lock();
  }
  bool ExecEngine::IsBusy() const {
    return queue ? queue->IsBusy() : isBusy;
  }
  const aliLuaCore::Exec::Listeners::Ptr &ExecEngine::OnIdle() const {
    return onIdle;
  }
  ExecEngine::~ExecEngine() {
    if (L) {
      std::lock_guard<std::recursive_mutex> g(runLock);
      lua_close(L);
      L=nullptr;
    }
  }
  ExecEngine::ExecEngine(const std::string &name_)
    : Exec(engineExecType, name_),
      L(luaL_newstate()) {
    lua_checkstack(L,1);
    lua_pushlightuserdata(L,this);
    aliLuaCore::Util::RegistrySet(L, engineKey);
  }
  std::ostream &operator<<(std::ostream &out, const ExecEngine &o) {
    return out << "ExecEngine(" << (const aliLuaCore::Exec&)o << ")";
  }

  void ExecEngine::InternalRun(const aliLuaCore::Future::Ptr &future,
			       const aliLuaCore::LuaFn       &luaFn) {
    if (queue) {
      aliSystem::Threading::Work::Ptr work = ExecEngineWork::Create(Stats(),
								    THIS,
								    luaFn,
								    future,
								    PrivateRun);
      queue->AddWork(work);
    } else {
      isBusy = true;
      {
	aliSystem::StatsGuard g2(Stats());
	PrivateRun(THIS.lock(), future, luaFn);
      }
      isBusy = false;
      onIdle->Notify(THIS);
    }
  }
  void ExecEngine::PrivateRun(const Ptr         &ePtr,
			      const aliLuaCore::Future::Ptr &future,
			      const aliLuaCore::LuaFn       &luaFn) {
    std::lock_guard<std::recursive_mutex> g1(ePtr->runLock);
    aliLuaCore::StackGuard                            g2(ePtr->L);
    try {
      luaFn(ePtr->L);
      if (future) {
	future->SetValue(aliLuaCore::Values::GetMakeFnForAll(ePtr->L));
      }
    } catch (std::exception &e) {
      if (future) {
	future->SetError(e.what());
      }
    }
  }

}
