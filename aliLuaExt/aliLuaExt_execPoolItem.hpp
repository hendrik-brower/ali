#ifndef INCLUDED_ALI_LUA_EXT_EXEC_POOL_ITEM
#define INCLUDED_ALI_LUA_EXT_EXEC_POOL_ITEM

#include <aliLuaCore.hpp>
#include <memory>
#include <string>

namespace aliLuaExt {

  /// @brief The ExecPoolItem is a helper class for defining work queued for a
  ///        ExecPool.
  struct ExecPoolItem {
    using Ptr = std::shared_ptr<ExecPoolItem>; ///< shared pointer

    /// @brief constructor
    /// @param future is an object to contain the function call's results
    /// @param luaFn is a function to execute
    ExecPoolItem(const aliLuaCore::Future::Ptr &future,
		 const aliLuaCore::LuaFn       &luaFn);
    
    /// @brief GetFuture retrieves the future maintained by this object
    /// @return the future
    const aliLuaCore::Future::Ptr &GetFuture() const;
    
    /// @brief GetLuaFn retrieves the Lua function maintained by this object
    /// @return the Lua function
    const aliLuaCore::LuaFn       &GetLuaFn () const;
    
  private:
    aliLuaCore::Future::Ptr future; ///< future
    aliLuaCore::LuaFn       luaFn;  ///< luaFn
  };

}

#endif
