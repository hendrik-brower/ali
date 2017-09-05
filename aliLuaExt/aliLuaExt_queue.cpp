#include <aliLuaExt_queue.hpp>
#include <aliSystem.hpp>

namespace {
  using OBJ  = aliLuaExt::Queue::OBJ;
  using LOBJ = aliLuaExt::Queue::LOBJ;

  // ****************************************************************************************
  // Queue API
  int Create(lua_State *L) {
    THROW_IF(!lua_istable(L,1), "Expecting a table argument");
    std::string name;
    aliLuaCore::Table::GetString(L, 1, "name", name, false);
    return OBJ::Make(L, aliLuaExt::Queue::Create(name));
  }
  int Name(lua_State *L) {
    aliLuaExt::Queue::Ptr ptr = OBJ::Get(L,1,false);
    return aliLuaCore::Values::MakeString(L, ptr->Name());
  }
  int Next(lua_State *L) {
    aliLuaExt::Queue::Ptr ptr = OBJ::Get(L,1,false);
    return ptr->PushNext(L);
  }
  int Append(lua_State *L) {
    aliLuaExt::Queue::Ptr     ptr  = OBJ::Get(L,1,false);
    ptr->Append(aliLuaCore::Values::GetMakeFnRemaining(L, 2));
    return 0;
  }
  int Insert(lua_State *L) {
    aliLuaExt::Queue::Ptr     ptr  = OBJ::Get(L,1,false);
    ptr->Insert(aliLuaCore::Values::GetMakeFnRemaining(L, 2));
    return 0;
  }
  int Empty(lua_State *L) {
    aliLuaExt::Queue::Ptr ptr = OBJ::Get(L,1,false);
    lua_checkstack(L,1);
    lua_pushboolean(L, ptr->Empty());
    return 1;
  }
  int Size(lua_State *L) {
    aliLuaExt::Queue::Ptr ptr = OBJ::Get(L,1,false);
    lua_checkstack(L,1);
    lua_pushinteger(L, ptr->Size());
    return 1;
  }
  int OnFirst(lua_State *L) {
    bool       useWeak;
    LOBJ::TPtr lPtr;
    OBJ::TPtr  ptr = OBJ::Get(L,1,false);
    aliLuaCore::Table::GetBool(L, 2, "useWeak",  useWeak, false);
    LOBJ::GetTableValue   (L, 2, "listener", lPtr,    false);
    aliLuaExt::Queue::Listeners::Register(ptr->OnFirst(), lPtr, useWeak);
    return 0;
  }
  int OnInsert(lua_State *L) {
    bool       useWeak;
    LOBJ::TPtr lPtr;
    OBJ::TPtr  ptr = OBJ::Get(L,1,false);
    aliLuaCore::Table::GetBool(L, 2, "useWeak",  useWeak, false);
    LOBJ::GetTableValue   (L, 2, "listener", lPtr,    false);
    aliLuaExt::Queue::Listeners::Register(ptr->OnInsert(), lPtr, useWeak);
    return 0;
  }
  // ****************************************************************************************
  // Queue::Listener API
  int CreateListener(lua_State *L) {
    using Listener = aliLuaExt::Queue::Listener;
    const static std::string name = "queue listener";
    aliLuaCore::Exec::Ptr       exec;
    aliLuaCore::Exec::WPtr      wExec;
    aliLuaCore::CallTarget::Ptr target;
    // FIXME: allow one to pass a weak or strong exec/target and hold it accordingly
    aliLuaCore::Exec      ::OBJ::GetTableValue(L, 1, "exec",   exec,   false);
    aliLuaCore::CallTarget::OBJ::GetTableValue(L, 1, "target", target, false);
    wExec = exec;
    Listener::NotifyFn fn = [=](const aliLuaExt::Queue::WPtr &wqPtr) {
      aliLuaCore::Exec::Ptr exec = wExec.lock();
      if (exec && !wqPtr.expired()) {
	aliLuaCore::MakeFn arg = OBJ::GetMakeWeakFn(wqPtr);
	target->Run(exec, arg);
      }
    };
    LOBJ::TPtr rtn = Listener::Create(name, fn);
    return LOBJ::Make(L,rtn);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("queue functions");
    fnMap->Add("Create", Create);
    fnMap->Add("CreateListener", CreateListener);
    aliLuaCore::FunctionMap::Ptr mtMapListener = aliLuaCore::FunctionMap::Create("queue listener MT");
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("queue MT");
    mtMap->Add("Name",     Name);
    mtMap->Add("Next",     Next);
    mtMap->Add("Append",   Append);
    mtMap->Add("Insert",   Insert);
    mtMap->Add("Empty",    Empty);
    mtMap->Add("Size",     Size);
    mtMap->Add("OnFirst",  OnFirst);
    mtMap->Add("OnInsert", OnInsert);
    OBJ::Init("luaQueue",          mtMap,         true);
    LOBJ::Init("luaQueueListener", mtMapListener, true);
    aliLuaCore::Module::Register("load luaQueue functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.queue",  fnMap);
			       OBJ::Register(ePtr);
			       LOBJ::Register(ePtr);
			     });
  }
  void Fini() {
    OBJ::Fini();
  }

}

namespace aliLuaExt {

  void Queue::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::Queue", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  Queue::Ptr Queue::Create(const std::string &name) {
    Ptr rtn(new Queue);
    rtn->THIS     = rtn;
    rtn->name     = name;
    rtn->onFirst  = Listeners::Create("onFirst");
    rtn->onInsert = Listeners::Create("onInsert");
    return rtn;
  }
  Queue::~Queue() {}
  const std::string &Queue::Name() const { return name; }
  int Queue::PushNext(lua_State *L) {
    std::lock_guard<std::mutex> g(lock);
    if (items.empty()) {
      return 0;
    } else {
      aliLuaCore::MakeFn item = items.front();
      items.pop_front();
      int rc = item(L);
      return rc;
    }
  }
  Queue::Ptr Queue::GetPtr() const {
    return THIS.lock();
  }
  void Queue::Append(const aliLuaCore::MakeFn &item) {
    THROW_IF(!item, "Attempt to append an uninitialized item");
    std::lock_guard<std::mutex> g(lock);
    items.push_back(item);
    Notify(items.size()-1);
  }
  void Queue::Insert(const aliLuaCore::MakeFn &item) {
    THROW_IF(!item, "Attempt to append an uninitialized item");
    std::lock_guard<std::mutex> g(lock);
    items.push_front(item);
    Notify(items.size()-1);
  }
  bool Queue::Empty() {
    std::lock_guard<std::mutex> g(lock);
    return items.empty();
  }
  size_t Queue::Size() {
    std::lock_guard<std::mutex> g(lock);
    return items.size();
  }
  const Queue::Listeners::Ptr &Queue::OnFirst () const { return onFirst;  }
  const Queue::Listeners::Ptr &Queue::OnInsert() const { return onInsert; }

  std::ostream &operator<<(std::ostream &out, const Queue &o) {
    return out << "Queue(name=" << o.Name() << ")";
  }

  Queue::Queue() {}
  void Queue::Notify(size_t sz) {
    if (!sz) {
      onFirst->Notify(THIS);
    }
    onInsert->Notify(THIS);
  }


}
