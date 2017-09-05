#ifndef INCLUDED_ALI_LUA_CORE
#define INCLUDED_ALI_LUA_CORE

#include <aliLuaCore_callTarget.hpp>
#include <aliLuaCore_deserialize.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliLuaCore_future.hpp>
#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_object.hpp>
#include <aliLuaCore_serialize.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_staticObject.hpp>
#include <aliLuaCore_stats.hpp>
#include <aliLuaCore_table.hpp>
#include <aliLuaCore_types.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_values.hpp>

/// @brief The aliLuaCore namespace defines the elements supporting
///        the aliLua* interface for advanced integration of C++ to
///        Lua.
namespace aliLuaCore {

  /// @brief RegisterInitFini triggers the registration of any
  ///        aliSystem::Components defined in the aliLuaCore library.
  /// @param cr is a component registry to which any initialzation
  ///        and finalization logic should be registered.
  void RegisterInitFini(aliSystem::ComponentRegistry &cr);
  
}

#endif
