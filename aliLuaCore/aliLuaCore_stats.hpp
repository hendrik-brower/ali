#ifndef INCLUDED_ALI_LUA_CORE_STATS
#define INCLUDED_ALI_LUA_CORE_STATS

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>

namespace aliLuaCore {

  /// @brief aliSystem:Stats provides a simple mechanism for tracking usage
  ///        as a number of occurances and a time/use. The aliLuaCore::Stats
  ///        exposes this object to Lua.
  struct Stats {

    /// @brief a static object for system stats
    using OBJ = StaticObject<aliSystem::Stats>;

    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief a aliSystem::Stats info function
    /// @param ptr the stats pointer to use for generating the info
    /// @return a MakeFn which builds a of stats information.
    static MakeFn Info(const aliSystem::Stats::Ptr &ptr);
    
  };

}

#endif
