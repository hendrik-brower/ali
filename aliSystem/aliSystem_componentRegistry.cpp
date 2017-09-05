#include <aliSystem_componentRegistry.hpp>
#include <aliSystem_logging.hpp>
#include <algorithm>
#include <iterator>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace {
  namespace locals {
    using Component = aliSystem::Component;
    using CR        = aliSystem::ComponentRegistry;

    void DoNothing() {}
    
    void SortMiddle(const CR::Vec &first, CR::Vec &middle);

    bool AllDepValid(const CR::SSet &srcSet, const CR::SSet &depSet) {
      bool valid = true;
      for (CR::SSet::const_iterator depIt=depSet.begin(); depIt!=depSet.end(); ++depIt) {
	std::string dep = *depIt;
	CR::SSet::const_iterator srcIt = srcSet.find(dep);
	if (srcIt==srcSet.end()) {
	  ERROR("No component provides " << dep);
	  valid = false;
	}
      }
      return valid;
    }
    bool HasDependent(const std::string &str, const CR::SSet &depSet) {
      return depSet.find(str)==depSet.end();
    }
    bool OrderByName(const Component::Ptr &a, const Component::Ptr &b) {
      THROW_IF(!a || !b, "Cannot order CItems if a or b is null");
      return a->Name() < b->Name();
    }

    void SortAll(CR::Vec        &components,
		 const CR::SSet &depSet) {
      CR::Vec first;
      CR::Vec last;
      CR::Vec middle;
      std::sort(components.begin(), components.end(), &OrderByName);
      //
      // split component list into 3 pieces:
      //    first -> components with no dependencies.
      //    middle -> compoennts with dependencies and for which
      //              other components depend.
      //    last -> components that have dependencies, but for which
      //            no other component depends.
      for (CR::Vec::const_iterator it=components.begin();
	   it!=components.end();
	   ++it) {
	const Component::Ptr &cPtr = *it;
	if (cPtr) {
	  if (cPtr->NumDep()==0) {
	    first.push_back(cPtr);
	  } else if (HasDependent(cPtr->Name(), depSet)) {
	    last.push_back(cPtr);
	  } else {
	    middle.push_back(cPtr);
	  }
	}
      }
      SortMiddle(first, middle);
      // rebuild components vector
      components.clear();
      std::copy(first .begin(), first .end(), std::back_inserter(components));
      std::copy(middle.begin(), middle.end(), std::back_inserter(components));
      std::copy(last  .begin(), last  .end(), std::back_inserter(components));
    }

    void SortMiddle(const CR::Vec &first, CR::Vec &middle) {
      using SPMap = std::unordered_multimap<std::string, Component::Ptr>;
      using Pair  = std::pair<SPMap::iterator, SPMap::iterator>;
      using SVec  = std::vector<std::string>;
      CR::SSet available;
      for (CR::Vec::const_iterator it=first.begin(); it!=first.end(); ++it) {
	const Component::Ptr ptr = *it;
	if (ptr) {
	  available.insert(ptr->Name());
	}
      }
      CR::Vec tmp = middle;
      SPMap spMap;
      middle.clear();
      for (CR::Vec::iterator tmpIt=tmp.begin(); tmpIt!=tmp.end(); ++tmpIt) {
	Component::Ptr ptr = *tmpIt;
	if (ptr) {
	  int ok = true;
	  const Component::SSet &depSet = ptr->GetDependencies();
	  for (Component::SSet::const_iterator depIt=depSet.begin(); depIt!=depSet.end(); ++depIt) {
	    std::string src = *depIt;
	    if (available.find(src)==available.end()) {
	      ok = false;
	      spMap.insert(SPMap::value_type(src, ptr));
	      // Deferring ptr->Name() << " for " << src
	      break; // just add the first unresoved dependency
	    }
	  }
	  if (ok) {
	    SVec addedVec;
	    // Adding " << ptr->Name()
	    middle.push_back(ptr);
	    available.insert(ptr->Name());
	    addedVec.push_back(ptr->Name());
	    for (size_t addedIdx=0;addedIdx<addedVec.size();++addedIdx) {
	      std::string added = addedVec[addedIdx];
	      // Checking any queued items that had a dependency for " << added
	      Pair pair = spMap.equal_range(added);
	      for (SPMap::iterator spMapIt=pair.first;
		   spMapIt!=pair.second && spMapIt!=spMap.end();
		   spMapIt = spMap.erase(spMapIt)) {
		Component::Ptr p = spMapIt->second;
		if (p) {
		  // Rechecking deferred " << p->Name()
		  ok = true;
		  const Component::SSet &depSet = p->GetDependencies();
		  for (Component::SSet::const_iterator depIt=depSet.begin(); depIt!=depSet.end(); ++depIt) {
		    std::string src = *depIt;
		    if (available.find(src)==available.end()) {
		      // Redeferring " << p->Name() << " for " << src
		      ok = false;
		      spMap.insert(SPMap::value_type(src, ptr));
		      break; // just add the first unresolved dependency
		    }
		  }
		  if (ok) {
		    //Adding deferred " << p->Name()
		    middle.push_back(p);
		    available.insert(p->Name());
		    addedVec.push_back(p->Name());
		  }
		}
	      }
	      // done with deferred
	    }
	  }
	}
      }
      for (SPMap::iterator spMapIt=spMap.begin(); spMapIt!=spMap.end(); ++spMapIt) {
	ERROR("Unresolved dependency for " << spMapIt->first << " for " << spMapIt->second->Name());
      }
      THROW_IF(!spMap.empty(), "Cyclical dependencies prevent component ordering");
    }

    void VerifyDependencies(const CR::Vec &components) {
      CR::SSet available;
      for (CR::Vec::const_iterator it=components.begin();
	   it!=components.end(); ++it) {
	Component::Ptr ptr = *it;
	if (ptr) {
	  const Component::SSet &depSet = ptr->GetDependencies();
	  for (CR::SSet::const_iterator it=depSet.begin(); it!=depSet.end(); ++it) {
	    std::string src = *it;
	    THROW_IF(available.find(src)==available.end(),
		     "dependency " << src << " for " << ptr->Name() << " is not defined in time");
	  }
	  available.insert(ptr->Name());
	}
      }
    }


    
  }
}

namespace aliSystem {

  ComponentRegistry::ComponentRegistry()
    : hasInit(false),
      hasFini(false) {
  }
  ComponentRegistry::~ComponentRegistry() {
    if (!hasFini) {
      Fini();
    }
  }
  Component::Ptr ComponentRegistry::Register(const std::string &name) {
    return Register(Component::Create(name, &locals::DoNothing, &locals::DoNothing));
  }
  Component::Ptr ComponentRegistry::Register(const std::string   &name,
					     const Component::Fn &initFn,
					     const Component::Fn &finiFn) {
    return Register(Component::Create(name, initFn, finiFn));
  }
  Component::Ptr ComponentRegistry::Register(const Component::Ptr &ptr) {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(!ptr, "Attempt to register an uninitialized component");
    THROW_IF(srcSet.find(ptr->Name())!=srcSet.end(),
	     "A component named " << ptr->Name() << " is already registered");
    THROW_IF(hasInit, "Attempt to add a component after the Init has been called");
    srcSet.insert(ptr->Name());
    components.push_back(ptr);
    return ptr;
  }
  bool ComponentRegistry::HasInit() const { return hasInit; }
  bool ComponentRegistry::HasFini() const { return hasFini; }
  void ComponentRegistry::GetComponents(Vec &vec) {
    std::lock_guard<std::mutex> g(lock);
    std::copy(components.begin(),
	      components.end(),
	      std::back_inserter(vec));
  }
  void ComponentRegistry::Init() {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(hasInit, "Attempt to re-run init");
    hasInit = true;
    SSet depSet;
    for (Vec::iterator it=components.begin(); it!=components.end(); ++it) {
      Component::Ptr ptr = *it;
      ptr->Freeze();
      ptr->GetDependencies(depSet);
    }
    THROW_IF(!locals::AllDepValid(srcSet, depSet), "undefined dependencies exist, aborting init");
    locals::SortAll(components, depSet);
    locals::VerifyDependencies(components);
    for (Vec::const_iterator it = components.begin();
	 it!=components.end();
	 ++it) {
      Component::Ptr ptr = *it;
      if (ptr) {
	const Component::Fn &initFn = ptr->GetInitFn();
	INFO("Init " << *ptr);
	initFn();
      }
    }
    INFO("Init complete");
  }
  void ComponentRegistry::Fini() {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(hasFini, "Attempt to re-run fini");
    hasFini = true;
    for (Vec::const_reverse_iterator it=components.rbegin();
	 it!=components.rend();
	 ++it) {
      Component::Ptr ptr = *it;
      if (ptr) {
	if (ptr->WasInit()) {
	  const Component::Fn &finiFn = ptr->GetFiniFn();
	  INFO("Fini " << *ptr);
	  finiFn();
	}
      }
    }
    INFO("Fini complete");
  }


  void ComponentRegistry::GetSrcSet(SSet &sSet) {
    std::lock_guard<std::mutex> g(lock);
    std::copy(srcSet.begin(), srcSet.end(), std::inserter(sSet, sSet.begin()));
  }
  void ComponentRegistry::GetDepSet(SSet &sSet) {
    std::lock_guard<std::mutex> g(lock);
    for (Vec::iterator it=components.begin(); it!=components.end(); ++it) {
      Component::Ptr ptr = *it;
      ptr->GetDependencies(sSet);
    }
  }

}

