#ifndef INCLUDED_ALI_LUA_EXT_EXEC_ENGINE_WORK
#define INCLUDED_ALI_LUA_EXT_EXEC_ENGINE_WORK

#include <aliLuaExt_execEngine.hpp>
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <functional>
#include <string>

namespace aliLuaExt {

  /// @brief The ExecEngineWork is a helper class for defining work queued for a
  ///        ExecEngine.
  struct ExecEngineWork {

    /// @brief THe run function wraps the primary elements of a Lua interpreter
    ///        call for this Lua wrapper library.
    using RunFn = std::function<void(const aliLuaExt::ExecEngine::Ptr  &,
				     const aliLuaCore::Future::Ptr     &,
				     const aliLuaCore::LuaFn           &)>;

    /// @brief Construct a work object for insertion into a Thread queue.
    /// @param statsPtr is the object on which execution stats for the call
    ///        will be recorded.
    /// @param wePtr is a weak pointer to the ExecEngine.
    /// @param fn is the functor to execute
    /// @param future is the call's results container
    /// @param runFn is the function to call when executing a work unit.
    static aliSystem::Threading::Work::Ptr Create(const aliSystem::Stats::Ptr       &statsPtr,
						  const aliLuaExt::ExecEngine::WPtr &wePtr,
						  const aliLuaCore::LuaFn           &fn,
						  const aliLuaCore::Future::Ptr     &future,
						  const RunFn                       &runFn);
  };

}

#endif
