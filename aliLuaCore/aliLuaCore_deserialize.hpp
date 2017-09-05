#ifndef INCLUDED_ALI_LUA_CORE_DESERIALIZE
#define INCLUDED_ALI_LUA_CORE_DESERIALIZE

#include <aliSystem.hpp>
#include <aliLuaCore_MT.hpp>
#include <iostream>

struct lua_State;
namespace aliLuaCore {

  /// @brief Deserialize defines a set of utility functions for
  ///        deserializing Lua object from streams or strings.
  /// @note All functions assume the string is encoded
  ///       in a compatible format.  On should generally
  ///       assume that they were encoded with the
  ///       peer Serialize or Serialize::Util structures'
  ///       API.
  struct Deserialize {
    using DObj = aliSystem::Codec::Deserializer;     ///< deserializer
    
    /// @brief Initialize Deserialize module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief extract the contents of the passed stream
    ///        to a Lua state, pushing elements onto the stack
    ///        as they are deserialized.
    /// @param L Lua state to push the extrated elements.
    /// @param dObj is the deserialize object to use to deserialize
    ///        the given stream.
    /// @return An int indicating how many items are left
    ///         on the stack by this routine.
    /// @note This function does not return the total number
    ///       of items pushed, only the ones left on the stack.
    /// @note If the stream does not contain recognized
    ///       elements, this function will throw an exception.
    /// @note The stream may include serialized objects defined
    ///       by aliLuaCore::MT.   The ability to extract these
    ///       objects is dependent upon the process decoding
    ///       decoding recognizing the object types as well
    ///       as having a compatible deserilization routine.
    static int ToLua(lua_State *L,
		     DObj      &dObj);
    
    /// @brief Create a make function that when run will extract
    ///        the contents of the passed stream to a Lua state,
    ///        pushing elements onto the stack as they are deserialized.
    /// @param dObj is the deserialize object to use to deserialize
    ///        the given stream.
    /// @return MakeFn that wraps the deserialization.
    /// @note If the stream does not contain recognized
    ///       elements, this function will throw an exception.
    ///       This exception will be thrown when the MakeFn is
    ///       run.
    /// @note The stream may include serialized objects defined
    ///       by aliLuaCore::MT.   The ability to extract these
    ///       objects is dependent upon the process decoding
    ///       decoding recognizing the object types as well
    ///       as having a compatible deserilization routine.
    static MakeFn GetMakeFn(DObj &dObj);

  };
}

#endif
