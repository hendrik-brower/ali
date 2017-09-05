#ifndef INCLUDED_ALI_LUA_EXT
#define INCLUDED_ALI_LUA_EXT

#include <aliLuaExt_codec.hpp>
#include <aliLuaExt_execEngine.hpp>
#include <aliLuaExt_execEngineWork.hpp>
#include <aliLuaExt_execPool.hpp>
#include <aliLuaExt_functionTarget.hpp>
#include <aliLuaExt_hold.hpp>
#include <aliLuaExt_IO.hpp>
#include <aliLuaExt_IOOptions.hpp>
#include <aliLuaExt_objectTarget.hpp>
#include <aliLuaExt_queue.hpp>
#include <aliLuaExt_threading.hpp>

/// @brief The aliLuaExt namespace defines the extension elements supporting
///        the ali interface to Lua.
namespace aliLuaExt {
  /// @brief RegisterInitFini triggers the registration of any
  ///        aliSystem::Components defined in the aliLua library.
  /// @param cr is a component registry to which any initialzation
  ///        and finalization logic should be registered.
  void RegisterInitFini(aliSystem::ComponentRegistry &cr);
}

#endif

