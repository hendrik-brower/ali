#include <aliSystem_registry.hpp>
#include <aliSystem_logging.hpp>

namespace aliSystem {

  Registry::Ptr Registry::Create(const std::string &name) {
    Ptr rtn(new Registry);
    rtn->name = name;
    return rtn;
  }
  const std::string &Registry::Name() const { return name; }
  void Registry::Register(const ItemPtr &ptr) {
    if (ptr.get()) {
      std::lock_guard<std::mutex> g(lock);
      items[ptr.get()] = ptr;
    }
  }
  void Registry::Unregister(const ItemPtr &ptr) {
    Unregister(ptr.get());
  }
  void Registry::Unregister(const void *addr) {
    std::lock_guard<std::mutex> g(lock);
    items.erase(addr);
  }
  Registry::ItemPtr Registry::Get(const void *addr) {
    ItemPtr rtn;
    std::lock_guard<std::mutex> g(lock);
    ItemMap::const_iterator it = items.find(addr);
    if (it!=items.end()) {
      rtn = it->second;
    }
    return rtn;
  }
  Registry::~Registry() {}
  Registry::Registry() {}

}
