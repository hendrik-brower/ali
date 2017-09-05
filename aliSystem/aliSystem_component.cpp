#include <aliSystem_component.hpp>
#include <aliSystem_logging.hpp>
#include <algorithm>
#include <iterator>

namespace aliSystem {

  Component::Ptr Component::Create(const std::string &name,
				   const Fn          &initFn,
				   const Fn          &finiFn) {
    Ptr rtn(new Component);
    WPtr wPtr = rtn;
    rtn->name = name;
    rtn->initFn = [=]() {
      Ptr ptr = wPtr.lock();
      if (ptr) {
	THROW_IF(ptr->wasInit, "Init function was already triggered");
	ptr->wasInit = true;
	initFn();
      }
    };
    rtn->finiFn = [=]() {
      Ptr ptr = wPtr.lock();
      if (ptr) {
	THROW_IF(!ptr->wasInit, "Init function was not triggered");
	THROW_IF( ptr->wasFini, "Fini function was already triggered");
	ptr->wasFini = true;
	finiFn();
      }
    };
    return rtn;
  }
    
  Component::~Component() {
  }
  const std::string   &Component::Name() const { return name; }
  const Component::Fn &Component::GetInitFn() const { return initFn; }
  const Component::Fn &Component::GetFiniFn() const { return finiFn; }
  bool                 Component::IsFrozen() const { return isFrozen; }
  bool                 Component::WasInit() const { return wasInit; }
  bool                 Component::WasFini() const { return wasFini; }
  size_t               Component::NumDep() {
    std::lock_guard<std::mutex> g(lock);
    return depSet.size();
  }
  const Component::SSet &Component::GetDependencies() const {
    THROW_IF(!isFrozen, "Attempt to get dependency set from a non-frozen component");
    return depSet;
  }
  void Component::GetDependencies(SSet &sSet) {
    std::lock_guard<std::mutex> g(lock);
    std::copy(depSet.begin(), depSet.end(), std::inserter(sSet, sSet.begin()));
  }

  void Component::Freeze() { isFrozen = true; }
  void Component::AddDependency(const std::string &str) {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(isFrozen, "Attempt to add a dependency (" << str
	     << ") to a frozen component: " << *this);
    THROW_IF(str==name, "A component cannot depend upon itself.  " << *this);
    depSet.insert(str);
  }

  std::ostream &operator<<(std::ostream &out, const Component &o) {
    out << "Component(" << o.name << ")";
    return out;
  }

  Component::Component()
    : isFrozen(false),
      wasInit(false),
      wasFini(false) {
  }
  
}
