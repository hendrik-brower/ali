#ifndef INCLUDED_ALI_LUA_EXT_CODEC
#define INCLUDED_ALI_LUA_EXT_CODEC

#include <aliSystem.hpp>
#include <aliLuaCore.hpp>
#include <iostream>

struct lua_State;
namespace aliLuaExt {

  /// @brief Codec defines a set of utility objects related
  ///        to serialization & deserialization.
  struct Codec {
    using DPtr = aliSystem::Codec::Deserialize::Ptr;                  ///< deserializer
    using SPtr = aliSystem::Codec::Serialize  ::Ptr;                  ///< serializer
    using DOBJ = aliLuaCore::StaticObject<aliSystem::Codec::Deserialize>; ///< static obj
    using SOBJ = aliLuaCore::StaticObject<aliSystem::Codec::  Serialize>; ///< static obj
    
    /// @brief Initialize Codec module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

  };
}

#endif
