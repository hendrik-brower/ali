#include <aliLuaExt_execEngineWork.hpp>

namespace aliLuaExt {
    
  aliSystem::Threading::Work::Ptr ExecEngineWork::Create(const aliSystem::Stats::Ptr       &stats,
							 const aliLuaExt::ExecEngine::WPtr &wePtr,
							 const aliLuaCore::LuaFn           &fn,
							 const aliLuaCore::Future::Ptr     &future,
							 const RunFn                       &runFn) {
    aliSystem::Threading::Work::WorkFn workFn = [=](bool &requeue) {
      requeue = false;
      aliLuaExt::ExecEngine::Ptr ePtr = wePtr.lock();
      if (ePtr) {
	runFn(ePtr, future, fn);
      } else {
	WARN("Released engine, dropping call");
      }
    };
    return aliSystem::Threading::Work::Create(stats, workFn);
  }
    
}
