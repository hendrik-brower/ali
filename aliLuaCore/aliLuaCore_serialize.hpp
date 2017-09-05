#ifndef INCLUDED_ALI_LUA_CORE_SERIALIZE
#define INCLUDED_ALI_LUA_CORE_SERIALIZE

#include <aliSystem.hpp>
#include <aliLuaCore_MT.hpp>
#include <iostream>

struct lua_State;
namespace aliLuaCore {
  
  /// @brief Serialize defines an interface for a set of utility 
  ///        functions for serializing Lua values to streams or
  ///        strings.
  struct Serialize {

    using SObj = aliSystem::Codec::Serializer;  ///< serializer

    /// @brief Write will encode the value at the given index of
    ///        the passed Lua State to the given stream.
    /// @param L Lua State containing the value.
    /// @param index is the stack index of the value to encode
    /// @param sObj is the aliSystem::Codec::Serializer object to
    ///        use to serialize the given stream.
    /// @note If the type cannot be encoded, then an exception will
    ///       be thrown.
    /// @note C++ types defined with aliLuaCore::MT may be serialized
    ///       if they are declared with a serialization function.
    static void Write(lua_State *L,
		      int        index,
		      SObj      &sObj);

    /// @brief Stream will encode the value(s) starting at the given
    ///        index and continuing for the given count (up until the
    ///        top of the stack) the passed Lua State to the given
    ///        stream.
    /// @param L Lua State containing the value(s) to serialize.
    /// @param index is the stack index of the first value to encode
    /// @param count is the number of items to encode.
    /// @param sObj is the aliSystem::Codec::Serializer object to
    ///        use to serialize the given stream.
    /// @note If the type of one of the targted values cannot be
    ///       encoded, then an exception will be thrown.
    /// @note C++ types defined with aliLuaCore::MT may be serialized
    ///       if they are declared with a serialization function.
    /// @note This function will not encode any value if the index
    ///       is of the type LUA_TNONE.
    /// @note This function may encode fewer than count values if
    ///       the top of the stack is reached before the given number
    ///       of items is serialized.   In this case, no exception
    ///       will be thrown.
    static void Write(lua_State *L,
		       int        index,
		       size_t     count,
		       SObj      &sObj);

  };
  
}

#endif
