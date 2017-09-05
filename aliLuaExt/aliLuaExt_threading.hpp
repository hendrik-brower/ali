#ifndef INCLUDED_ALI_LUA_EXT_THREADING
#define INCLUDED_ALI_LUA_EXT_THREADING

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {
  namespace Threading {
    
    /// @brief Initialize Threading module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief The PoolOBJ is a utilty object for access/manipulations of
    ///        aliSystem::Threading::Pool objects.
    using PoolOBJ  = aliLuaCore::StaticObject<aliSystem::Threading::Pool>;

    /// @brief The QueueOBJ is a utilty object for access/manipulations of
    ///        aliSystem::Threading::Queue objects.
    using QueueOBJ = aliLuaCore::StaticObject<aliSystem::Threading::Queue>;

    /// @brief The WorkOBJ is a utilty object for access/manipulations of
    ///        aliSystem::Threading::Work objects.
    using WorkOBJ  = aliLuaCore::StaticObject<aliSystem::Threading::Work>;
    
  }
}


#endif
