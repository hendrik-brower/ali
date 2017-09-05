#ifndef INCLUDED_ALI_LUA_EXT_IO
#define INCLUDED_ALI_LUA_EXT_IO

#include <aliSystem.hpp>
#include <aliLuaExt_IOOptions.hpp>
#include <ostream>
#include <string>

struct lua_State;
namespace aliLuaExt {

  /// @brief The IO structure is used to faciliate serializing values
  ///        from the Lua stack.
  ///
  /// Its typical use would be something of the form:
  ///     ostream << IO(L,opt);
  /// where ostream is the stream into which the values from L will
  /// be written.
  ///
  /// IOOptions provides a means of specializing the behavior of the
  /// IO object.
  struct IO {

    /// @brief Initialize IO module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Log provides a simple logging mechanism to generate a
    ///        log for the current Lua Stack.  When this method is
    ///        called, all values on the stack will be logged.
    ///
    /// This function primarily targets debugging uses or generic
    /// logging from Lua a call to this function is registered
    /// for Exec interpreters.
    ///
    /// @param L is the Lua State from which values should be logged.
    /// @return This function should always return 0 since it does
    ///         not push any values onto the Lua Stack.
    static int Log(lua_State *L);
    
    /// @brief constructor
    /// @param L is the Lua interpreter
    explicit
    IO(lua_State *L);
    
    /// @brief constructor
    /// @param L is the Lua interpreter
    /// @param index is the stack index of the value to stream
    /// @note This funtion will only stream 0 or 1 values.  Zero
    ///       values are only streamed if the index is an LUA_TNONE.
    IO(lua_State *L, int index);
    
    /// @brief constructor
    /// @param L is the Lua interpreter
    /// @param opt is the options to assocate with the created
    ///        instance
    IO(lua_State *L, const IOOptions &opt);
    
    /// @brief GetOptions will return a copy of the objects
    ///        options.
    /// @return options
    const IOOptions &GetOptions() const;
    
    /// @brief IO Stream operator
    /// @param out stream to which data should be serialized
    /// @param o IO instance to serialize
    /// @return the original stream object
    friend std::ostream &operator<<(std::ostream &out, const IO &o);
    
  private:
    lua_State *L;   ///< Lua State
    IOOptions  opt; ///< options
  };

}

#endif
