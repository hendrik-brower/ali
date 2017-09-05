#include <aliLuaCore_module.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace {
  struct Item {
    using Ptr  = std::shared_ptr<Item>;
    using IVec = std::vector<Ptr>;
    Item(const std::string name_, aliLuaCore::Module::ExecFn fn_)
      : name(name_),
	fn(fn_) {
    }
    const std::string &Name() const { return name; }
    void Run(const aliLuaCore::Exec::Ptr &ePtr) {
      fn(ePtr);
    }
  private:
    std::string                name;
    aliLuaCore::Module::ExecFn fn;
  };
  std::mutex lock;
  Item::IVec items;

  void Init() {}
  void Fini() {
    std::lock_guard<std::mutex> g(lock);
    items.clear();
  }

}

namespace aliLuaCore {

  void Module::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::Module", Init, Fini);
    ptr->AddDependency("aliSystem");
  }
  
  void Module::Register(const std::string &name, ExecFn execFn) {
    std::lock_guard<std::mutex> g(lock);
    Item::Ptr ptr(new Item(name, execFn));
    items.push_back(ptr);
  }
  void Module::InitEngine(const Exec::Ptr &ePtr) {
    THROW_IF(!ePtr, "Attempt to initialize modules with an uninitialized engine");
    std::lock_guard<std::mutex> g(lock);
    for (Item::IVec::iterator it=items.begin();
	 it!=items.end(); ++it) {
      Item::Ptr ptr = *it;
      ptr->Run(ePtr);
    }
  }

}
