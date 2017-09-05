#ifndef INCLUDED_ALI_LUA_EXT2
#define INCLUDED_ALI_LUA_EXT2

#include <aliLuaExt2_action.hpp>

/// @brief The aliLuaExt2 namespace defines the extension elements for
///        extending the capabilities of the ali interface to Lua.
namespace aliLuaExt2 {

  /// @brief RegisterInitFini triggers the registration of any
  ///        aliSystem::Components defined in the aliLua library.
  /// @param cr is a component registry to which any initialzation
  ///        and finalization logic should be registered.
  void RegisterInitFini(aliSystem::ComponentRegistry &cr);
  
}

#endif

