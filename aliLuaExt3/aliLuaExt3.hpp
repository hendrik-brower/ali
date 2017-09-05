#ifndef INCLUDED_ALI_LUA_EXT3
#define INCLUDED_ALI_LUA_EXT3

#include <aliLuaExt3_externalMT.hpp>
#include <aliLuaExt3_externalObject.hpp>

namespace aliSystem {
  struct ComponentRegistry;
}

/// @brief The aliLuaExt3 namespace defines the extension elements for
///        extending the capabilities of the ali interface to Lua.
namespace aliLuaExt3 {
  /// @brief RegisterInitFini triggers the registration of any
  ///        aliSystem::Components defined in the aliLua library.
  /// @param cr is a component registry to which any initialzation
  ///        and finalization logic should be registered.
  void RegisterInitFini(aliSystem::ComponentRegistry &cr);
}

#endif

