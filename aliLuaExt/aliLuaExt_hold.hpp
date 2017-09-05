#ifndef INCLUDED_ALI_LUA_EXT_HOLD
#define INCLUDED_ALI_LUA_EXT_HOLD

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {

  /// @brief Hold provides a Lua interface for aliSystem::Hold
  ///        objects.
  struct Hold {
    
    /// @brief A Static object wrapping a aliSystem::Hold
    using OBJ = aliLuaCore::StaticObject<aliSystem::Hold>;

    /// @brief Initialize Hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

  };
  
}

#endif
