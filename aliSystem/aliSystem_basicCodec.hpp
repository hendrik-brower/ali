#ifndef INCLUDED_ALI_SYSTEM_BASIC_CODEC
#define INCLUDED_ALI_SYSTEM_BASIC_CODEC

#include <aliSystem_codecDeserialize.hpp>
#include <aliSystem_codecSerialize.hpp>
#include <aliSystem_componentRegistry.hpp>

namespace aliSystem {

  /// @brief BasicCodec defines interfaces for built-in
  ///        serialization and deserialization implementations.
  /// @note In many cases, these built in serializer/deserialize
  ///       utilities will work just fine.  However, one should
  ///       be cautious of their use if their use will be a critical
  ///       aspect of their overall system.
  struct BasicCodec {

    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliSystem::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);


    /// @brief Retrieve a basic serialization object
    /// @return a serialize pointer
    /// @note The returned encoder is does not aim to provide optimally
    ///       compact output or the most efficient encoding/decoding.
    static Codec::Serialize  ::Ptr GetSerializer();
    
    /// @brief Retrieve a basic deserialization object
    /// @return a deserialize pointer
    /// @note This object works with the serializer returned by
    ///       GetSerializer.
    static Codec::Deserialize::Ptr GetDeserializer();

  };
  
}

#endif
