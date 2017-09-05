#include <aliLuaCore_functionMap.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliSystem.hpp>

namespace aliLuaCore {

  namespace {
    int DoNothing(lua_State *) {
      return 0;
    }
  }
  
  FunctionMap::Ptr FunctionMap::Create(const std::string &name) {
    Ptr rtn(new FunctionMap);
    rtn->name     = name;
    rtn->isFrozen = false;
    return rtn;
  }
  FunctionMap::~FunctionMap() {}
  const std::string &FunctionMap::Name() const { return name; }
  bool FunctionMap::IsFrozen() const { return isFrozen; }
  bool FunctionMap::IsDefined(const std::string &fnName) {
    std::lock_guard<std::mutex> g(lock);
    return fMap.find(fnName)!=fMap.end();
  }

  void FunctionMap::Freeze() {
    std::lock_guard<std::mutex> g(lock);
    isFrozen = true;
  }
  void FunctionMap::ForEach(const FFn &fn) {
    std::lock_guard<std::mutex> g(lock);
    bool more = true;
    for (FMap::iterator it=fMap.begin(); more && it!=fMap.end(); ++it) {
      more = fn(it->first, it->second);
    }
  }
  void FunctionMap::Add(const std::string &fnName, const LuaFn &fn) {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(isFrozen, "Cannot add functions to the frozen FunctionMap " << name);
    FMap::iterator it = fMap.find(fnName);
    THROW_IF(it!=fMap.end(), fnName << " is already defined in " << name);
    fMap[fnName] = fn;
  }
  void FunctionMap::Alias(const std::string &alias, const std::string &orig) {
    LuaFn fn = DoNothing;
    {
      std::lock_guard<std::mutex> g(lock);
      FMap::iterator it = fMap.find(orig);
      fn = it->second;
    }
    Add(alias, fn);
  }

  void FunctionMap::Wrap(const std::string &fnName, const WrapFn &wrapFn) {
    LuaFn dummy;
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(isFrozen, "Cannot wrap " << fnName << " in the frozen FunctionMap " << name);
    FMap::iterator it = fMap.find(fnName);
    if (it==fMap.end()) {
      fMap[fnName] = Functions::Wrap(dummy, wrapFn);
    } else {
      fMap[fnName] = Functions::Wrap(it->second, wrapFn);
    }
  }
  FunctionMap::FunctionMap() {}
  
}
