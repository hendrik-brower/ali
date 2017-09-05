#include <aliSystem_hold.hpp>
#include <aliSystem_componentRegistry.hpp>
#include <aliSystem_hold.hpp>
#include <aliSystem_logging.hpp>
#include <chrono>
#include <condition_variable>
#include <unordered_map>

namespace {  
  using HMap = std::unordered_map<const void*, aliSystem::Hold::WPtr>;
  std::condition_variable cv;
  std::mutex              lock;
  HMap                    holds;
  bool                    isInit   = false;
  bool                    released = false;

  void Init() { isInit = true; }
  void Fini() {}
  
  bool Release() {
    if (!released) {
      std::lock_guard<std::mutex> g(lock);
      released = isInit && holds.empty();
    }
    return released;
  }
}


namespace aliSystem {
  void Hold::RegisterInitFini(ComponentRegistry &cr) {
    cr.Register("aliSystem::Hold", Init, Fini);
  }
    
  Hold::Ptr Hold::Create(const std::string &file, size_t line, const std::string &reason) {
    std::lock_guard<std::mutex> g(lock);
    THROW_IF(released, "Attempt to acquire a hold after release");
    Ptr rtn(new Hold);
    holds[rtn.get()] = rtn;
    rtn->file = file;
    rtn->line = line;
    rtn->reason = reason;
    return rtn;
  }
  void Hold::WaitForHolds() {
    static std::chrono::seconds  tenSeconds(10);
    static std::mutex            waitMutex;
    std::unique_lock<std::mutex> lock(waitMutex);
    Vec vec;
    while (!Release()) {
      cv.wait_for(lock, tenSeconds, Release);
      GetHolds(vec);
      for (Vec::iterator it=vec.begin(); it!=vec.end(); ++it) {
	Ptr hPtr = *it;
	DEBUG("Waiting for " << hPtr->GetReason());
      }
      vec.clear();
    }
  }
  void Hold::GetHolds(Vec &vec) {
    std::lock_guard<std::mutex> g(lock);
    for (HMap::iterator it=holds.begin(); it!=holds.end(); ++it) {
      Ptr hold = it->second.lock();
      if (hold) {
	vec.push_back(hold);
      }
    }
  }
  const std::string &Hold::GetFile  () const { return file; }
  size_t             Hold::GetLine  () const { return line; }
  const std::string &Hold::GetReason() const { return reason; }
  Hold::~Hold() {
    std::lock_guard<std::mutex> g(lock);
    holds.erase(this);
    if (isInit && holds.empty()) {
      cv.notify_all();
    }
  }
  Hold::Hold() {}
  
}
