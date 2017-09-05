#include <aliLuaExt_execPool.hpp>
#include <aliLuaExt_execEngine.hpp>
#include <aliLuaExt_threading.hpp>
#include <sstream>

namespace {

  const std::string poolExecType = "execPool";
  using OBJ  = aliLuaExt::ExecPool::OBJ;
  using Pool = aliSystem::Threading::Pool;

  int CreatePool(lua_State *L) {
    std::string name;
    Pool::Ptr   pool;
    int         numEngines;
    aliLuaCore::Table::GetString (L,1, "name",       name, false);
    aliLuaCore::Table::GetInteger(L,1, "numEngines", numEngines, false);
    aliLuaExt::Threading::PoolOBJ::GetTableValue(L,1,"threadPool", pool, false);
    OBJ::TPtr ptr = aliLuaExt::ExecPool::Create(name, numEngines, pool);
    return OBJ::Make(L,ptr);
  }
  int Register(lua_State *L) {
    OBJ::TPtr         pool = OBJ::Get(L,1,false);
    aliLuaCore::Exec::Ptr exec = aliLuaCore::Exec::OBJ::Get(L,2,false);
    pool->Register(exec);
    return 0;
  }
  int Unregister(lua_State *L) {
    OBJ::TPtr         pool = OBJ::Get(L,1,false);
    aliLuaCore::Exec::Ptr exec = aliLuaCore::Exec::OBJ::Get(L,2,false);
    pool->Unregister(exec);
    return 0;
  }
  
  int GetInfo(lua_State *L) {
    OBJ::TPtr ptr = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn info = ptr->aliLuaCore::Exec::GetInfo();
    return info(L);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("exec pool functions");
    fnMap->Add("CreatePool", CreatePool);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("exec pool MT");
    mtMap->Add("Register",   Register);
    mtMap->Add("Unregister", Unregister);
    mtMap->Add("GetInfo",    GetInfo);
    OBJ::Init("luaExecPool", mtMap, true);
    aliLuaCore::MT::Ptr execMT = aliLuaCore::Exec::OBJ::GetMT();
    THROW_IF(!execMT, "Exec uninitialized");
    execMT->AddDerived(OBJ::GetMT());
    aliLuaCore::Module::Register("load aliLuaExt::ExecPool functions",
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

  void ExecPool::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::ExecPool", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }
  
  ExecPool::Ptr ExecPool::Create(const std::string &name,
				 size_t             numEngines,
				 const Pool::Ptr   &poolPtr) {
    THROW_IF(numEngines>50, "50 engines?  That just seems too high");
    THROW_IF(!poolPtr, "Must supply a threading pool");
    Ptr  rtn(new ExecPool(name));
    WPtr wRtn = rtn;
    rtn->THIS     = rtn;
    rtn->poolPtr  = poolPtr;
    rtn->onIdle   = Listeners::Create(name + ":onIdle");
    rtn->SetInfo("poolInfo", [=](lua_State *L) {
	aliLuaCore::MakeFn fn = GetInfoDetailed(wRtn.lock());
	return fn(L);
      });
    for (size_t i=0;i<numEngines;++i) {
      std::stringstream ss;
      ss << "engine " << i << " " << name;
      ExecEngine::Ptr ePtr = ExecEngine::Create(ss.str(), poolPtr);
      ePtr->SetInfo("poolInfo", [=](lua_State *L) {
	  aliLuaCore::MakeFn fn = GetInfoSimple(wRtn.lock());
	  return fn(L);
	});
      rtn->Register(ePtr);
    }
    return rtn;
  }
  aliLuaCore::MakeFn ExecPool::GetInfo(bool simple,
				       const aliLuaExt::ExecPool::Ptr &ptr) {
    THROW_IF(!ptr, "Uninitialized pointer");
    aliLuaCore::MakeTableUtil tbl;
    tbl.SetString ("name",           ptr->Name());
    tbl.SetBoolean("isBusy",         ptr->IsBusy());
    tbl.SetNumber ("numPending", (int)ptr->NumPending());
    tbl.SetString ("execType",        poolExecType);
    tbl.SetMakeFn ("pool",            ExecPool::OBJ::GetMakeWeakFn(ptr));
    if (!simple) {
      aliLuaCore::MakeTableUtil::Ptr itemsTbl = tbl.CreateSubtable("execItems");
      std::lock_guard<std::mutex> g(ptr->lock);
      int index = 0;
      for (ExecPool::ExecMap::iterator it=ptr->execMap.begin();
	   it!=ptr->execMap.end();
	   ++it) {
	aliLuaCore::Exec::Ptr ePtr = it->second;
	itemsTbl->SetMakeFnForIndex(++index, ePtr->GetInfo());
      }
    }
    return tbl.GetMakeFn();
  }
  aliLuaCore::MakeFn ExecPool::GetInfoDetailed(const ExecPool::Ptr &ptr) {
    return GetInfo(false, ptr);
  }
  aliLuaCore::MakeFn ExecPool::GetInfoSimple(const ExecPool::Ptr &ptr) {
    return GetInfo(true, ptr);
  }
  aliLuaCore::Exec::Ptr ExecPool::GetExec() const {
    return THIS.lock();
  }
  bool ExecPool::IsBusy() const { return isBusy; }
  const aliLuaCore::Exec::Listeners::Ptr &ExecPool::OnIdle() const {
    return onIdle;
  }
  void ExecPool::Register(const aliLuaCore::Exec::Ptr &ePtr) {
    Listener::Ptr  lPtr;
    THROW_IF(!ePtr, "attempt to register an uninitialized pointer");
    {
      std::lock_guard<std::mutex> g(lock);
      Exec::WPtr                  exec = ePtr;
      const void                 *execAddr = ePtr.get();
      Listener::NotifyFn          notifyFn;
      THROW_IF(execMap.find(execAddr)!=execMap.end(), "Attempt to re-register " << ePtr->Name());
      THROW_IF(execAddr==this, "Attempt to register the exec pool");
      execMap[execAddr] = ePtr;
      notifyFn = [=](const aliLuaCore::Exec::WPtr &) {
	ExecPool::Ptr epPtr = THIS.lock();
	if (epPtr) {
	  epPtr->Post(execAddr);
	}
      };
      lPtr = Listeners::Register(ePtr->OnIdle(),
				 "execIdle",
				 notifyFn,
				 true);
      listenerMap[execAddr] = lPtr;
      if (ePtr->IsBusy()) {
	// release if the registered exec is busy
	lPtr.reset();
      }
    }
    if (lPtr) { lPtr->Notify(THIS); }
  }
  void ExecPool::Unregister(const Exec::Ptr &ePtr) {
    std::lock_guard<std::mutex> g(lock);
    const void *execAddr = ePtr.get();
    if (execAddr) {
      execMap.erase(execAddr);
      idle.erase(execAddr);
      LMap::iterator it = listenerMap.find(execAddr);
      if (it!=listenerMap.end()) {
	listenerMap.erase(it);
      }
    }
  }
  void ExecPool::GetRegistered(ExecVec &vec) {
    std::lock_guard<std::mutex> g(lock);
    vec.clear();
    for (ExecMap::iterator it=execMap.begin(); it!=execMap.end(); ++it) {
      vec.push_back(it->second);
    }
  }
  size_t ExecPool::NumPending() {
    std::lock_guard<std::mutex> g(lock);
    return itemQueue.size();
  }
  
  ExecPool::~ExecPool() {}

  std::ostream &operator<<(std::ostream &out, const ExecPool &o) {
    out <<  "ExecPool(" << o.Name() << ")";
    return out;
  }
  void ExecPool::InternalRun(const aliLuaCore::Future::Ptr &future,
			     const aliLuaCore::LuaFn       &luaFn) {
    std::lock_guard<std::mutex> g(lock);
    while (!idle.empty()) {
      const void *exec = *idle.begin();
      idle.erase(exec);
      ExecMap::iterator it = execMap.find(exec);
      THROW_IF(it==execMap.end(), "engine not found");
      aliLuaCore::Exec::Ptr ePtr = it->second;
      if (!ePtr->IsBusy()) {
	ePtr->Run(future, luaFn);
	SetIsBusy(g);
	return;
      }
    }
    ExecPoolItem::Ptr iPtr(new ExecPoolItem(future,luaFn));
    itemQueue.push_back(iPtr);
    SetIsBusy(g);
  }
  ExecPool::ExecPool(const std::string &name)
    : Exec(poolExecType,  name),
      isBusy(false) {
  }
  void ExecPool::Post(const void *execAddr) {
    Listeners::Ptr    lPtr;
    std::string       pool = "unknown";
    ExecPoolItem::Ptr item;
    pool = Name();
    std::lock_guard<std::mutex> g(lock);
    ExecMap::iterator it = execMap.find(execAddr);
    if (it!=execMap.end()) {
      aliLuaCore::Exec::Ptr ePtr     = it->second;
      bool      haveItem = !itemQueue.empty();
      bool      eBusy    = ePtr->IsBusy();
      if (haveItem) {
	idle.erase(execAddr);
	if (!eBusy) {
	  item = itemQueue.front();
	  itemQueue.pop_front();
	  ePtr->Run(item->GetFuture(), item->GetLuaFn());
	}
      } else {
	if (eBusy) {
	  idle.erase(execAddr);
	} else {
	  idle.insert(execAddr);
	  lPtr = SetIsBusy(g);
	}
      }
    }
    if (lPtr) {
      lPtr->Notify(THIS);
    }
  }
  aliLuaCore::Exec::Listeners::Ptr ExecPool::SetIsBusy(std::lock_guard<std::mutex> &) {
    Listeners::Ptr lPtr;
    bool wasBusy = isBusy;
    isBusy = idle.empty();
    if (wasBusy && !isBusy) {
      lPtr = onIdle;
    }
    return lPtr;
  }


}

