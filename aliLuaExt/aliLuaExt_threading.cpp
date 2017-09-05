#include <aliLuaExt_threading.hpp>
#include <aliSystem.hpp>

namespace {

  using PoolOBJ  = aliLuaExt::Threading::PoolOBJ;
  using QueueOBJ = aliLuaExt::Threading::QueueOBJ;
  using WorkOBJ  = aliLuaExt::Threading::WorkOBJ;

  // ****************************************************************************************
  // Threading pool
  int Pool_Create(lua_State *L) {
    std::string           name;
    int                   numThreads = 0;
    aliSystem::Stats::Ptr stats;
    aliLuaCore::Table::GetString       (L, 1, "name",       name,       false);
    aliLuaCore::Table::GetInteger      (L, 1, "numThreads", numThreads, false);
    aliLuaCore::Stats::OBJ::GetTableValue(L, 1, "stats", stats, true);
    THROW_IF(numThreads<0,
	     "numThreads must be greater than or equal to zero, passed " << numThreads);
    PoolOBJ::TPtr ptr = aliSystem::Threading::Pool::Create(name,numThreads, stats);
    return PoolOBJ::Make(L, ptr);
  }
  int Pool_Flush(lua_State *L) {
    PoolOBJ::TPtr ptr = PoolOBJ::Get(L,1,false);
    ptr->Flush();
    return 0;
  }
  int Pool_Stop(lua_State *L) {
    PoolOBJ::TPtr ptr  = PoolOBJ::Get(L,1,false);
    bool      wait = lua_toboolean(L,2);
    ptr->Stop(wait);
    return 0;
  }
  int Pool_SetNumThreads(lua_State *L) {
    PoolOBJ::TPtr ptr  = PoolOBJ::Get(L,1,false);
    int       num  = lua_tointeger(L,2);
    ptr->SetNumThreads(num);
    return 0;
  }
  int Pool_GetInfo(lua_State *L) {
    PoolOBJ::TPtr                  ptr = PoolOBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil      rtn;
    rtn.SetString("name",       ptr->Name());
    rtn.SetNumber("numThreads", (int)ptr->GetNumThreads());
    rtn.SetMakeFn("stats",      aliLuaCore::Stats::OBJ::GetMakeFn(ptr->GetStats()));
    return rtn.GetMakeFn()(L);
  }
  int Pool_AddQueue(lua_State *L) {
    std::string           queueName;
    int                   maxConcurrency;
    aliSystem::Stats::Ptr stats;
    PoolOBJ::TPtr             ptr = PoolOBJ::Get(L,1,false);
    aliLuaCore::Table::GetString       (L, 2, "queueName",      queueName,      false);
    aliLuaCore::Table::GetInteger      (L, 2, "maxConcurrency", maxConcurrency, false);
    aliLuaCore::Stats::OBJ::GetTableValue(L, 2, "stats",          stats,          false);
    QueueOBJ::TPtr     queue = ptr->AddQueue(queueName, maxConcurrency, stats);
    return QueueOBJ::Make(L,queue);;
  }

  // ****************************************************************************************
  // Threading Queue
  int Queue_GetInfo(lua_State *L) {
    QueueOBJ::TPtr        ptr = QueueOBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil rtn;
    rtn.SetMakeFn("queueStats",   aliLuaCore::Stats::OBJ::GetMakeFn(ptr->QueueStats  ()));
    rtn.SetMakeFn("stoppedStats", aliLuaCore::Stats::OBJ::GetMakeFn(ptr->StoppedStats()));
    rtn.SetString ("name",                    ptr->Name());
    rtn.SetBoolean("isBusy",                  ptr->IsBusy());
    rtn.SetBoolean("isStopped",               ptr->IsStopped());
    rtn.SetBoolean("isFrozen",                ptr->IsFrozen());
    rtn.SetNumber ("currentConcurrency", (int)ptr->CurrentConcurrency());
    rtn.SetNumber ("maxConcurrency",     (int)ptr->GetMaxConcurrency());
    rtn.SetNumber ("numberPending",      (int)ptr->NumPending());
    return rtn.GetMakeFn()(L);
  }
  int Queue_Freeze(lua_State *L) {
    QueueOBJ::TPtr queue = QueueOBJ::Get(L,1,false);
    queue->Freeze();
    return 0;
  }
  int Queue_SetMaxConcurrency(lua_State *L) {
    QueueOBJ::TPtr queue = QueueOBJ::Get(L,1,false);
    int       val   = lua_tointeger(L,2);
    queue->SetMaxConcurrency(val);
    return 0;
  }
  int Queue_AddWork(lua_State *L) {
    QueueOBJ::TPtr     queue = QueueOBJ::Get(L,1,false);
    WorkOBJ::TPtr work  = WorkOBJ::Get(L,2,false);
    queue->AddWork(work);
    return 0;
  }
  int Queue_Stop(lua_State *L) {
    QueueOBJ::TPtr queue = QueueOBJ::Get(L,1,false);
    queue->Stop();
    return 0;
  }
  int Queue_StopAfter(lua_State *L) {
    QueueOBJ::TPtr     queue = QueueOBJ::Get(L,1,false);
    WorkOBJ::TPtr work  = WorkOBJ::Get(L,2,false);
    queue->StopAfter(work);
    return 0;
  }
  int Queue_Start(lua_State *L) {
    QueueOBJ::TPtr     queue = QueueOBJ::Get(L,1,false);
    queue->Start();
    return 0;
  }
  int Queue_Clear(lua_State *L) {
    QueueOBJ::TPtr     queue = QueueOBJ::Get(L,1,false);
    queue->Clear();
    return 0;
  }
  


  // ****************************************************************************************
  // Threading Work
  int Work_GetInfo(lua_State *L) {
    WorkOBJ::TPtr         ptr = WorkOBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil rtn;
    rtn.SetMakeFn("stats", aliLuaCore::Stats::OBJ::GetMakeFn(ptr->GetStats()));
    return rtn.GetMakeFn()(L);
  }
  
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("threading functions");
    fnMap->Add("CreatePool",  Pool_Create);
    aliLuaCore::FunctionMap::Ptr pool_mtMap = aliLuaCore::FunctionMap::Create("threading pool MT");
    pool_mtMap->Add("__tostring",    PoolOBJ::ToString);
    pool_mtMap->Add("Flush",         Pool_Flush);
    pool_mtMap->Add("Stop",          Pool_Stop);
    pool_mtMap->Add("SetNumThreads", Pool_SetNumThreads);
    pool_mtMap->Add("GetInfo",       Pool_GetInfo);
    pool_mtMap->Add("AddQueue",      Pool_AddQueue);
    aliLuaCore::FunctionMap::Ptr queue_mtMap = aliLuaCore::FunctionMap::Create("threading queue MT");
    queue_mtMap->Add("__tostring",        QueueOBJ::ToString);
    queue_mtMap->Add("GetInfo",           Queue_GetInfo);
    queue_mtMap->Add("Freeze",            Queue_Freeze);
    queue_mtMap->Add("SetMaxConcurrency", Queue_SetMaxConcurrency);
    queue_mtMap->Add("AddWork",           Queue_AddWork);
    queue_mtMap->Add("Stop",              Queue_Stop);
    queue_mtMap->Add("StopAfter",         Queue_StopAfter);
    queue_mtMap->Add("Start",             Queue_Start);
    queue_mtMap->Add("Clear",             Queue_Clear);
    aliLuaCore::FunctionMap::Ptr work_mtMap = aliLuaCore::FunctionMap::Create("threading work MT");
    work_mtMap->Add("__tostring",    WorkOBJ::ToString);
    work_mtMap->Add("GetInfo",       Work_GetInfo);

    PoolOBJ ::Init("luaThreadPool",  pool_mtMap,  true);
    QueueOBJ::Init("luaThreadQueue", queue_mtMap, true);
    WorkOBJ ::Init("luaThreadWork",  work_mtMap,  true);
    aliLuaCore::Module::Register("load aliLuaExt::Thread functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.threading",  fnMap);
			       PoolOBJ ::Register(ePtr);
			       QueueOBJ::Register(ePtr);
			       WorkOBJ ::Register(ePtr);
			     });
  }
  void Fini() {
    PoolOBJ ::Fini();
    QueueOBJ::Fini();
    WorkOBJ ::Fini();
  }

}

namespace aliLuaExt {

  void Threading::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::Threading", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  
}
