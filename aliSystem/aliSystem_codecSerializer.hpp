#ifndef INCLUDED_ALI_SYSTEM_CODEC_SERIALIZER
#define INCLUDED_ALI_SYSTEM_CODEC_SERIALIZER

#include <aliSystem_codecSerialize.hpp>
#include <string>
#include <iostream>

namespace aliSystem {
  namespace Codec {

    /// @brief Serializer provides a set of serialization
    ///        API's for encoding common basic types to a
    ///        a stream.
    /// @note Since this object holds a reference to an ostream
    ///       one should be careful with the life cycle of these
    ///       objects.  The life of the ostream should exceed
    ///       the life of any referencing Serializer.
    /// @note This class allows coding serialization without
    ///       direct access to the underlying stream, which
    ///       helps ensure that the encoding is completely
    ///       separated away from the encoding of the content.
    struct Serializer {

      /// @brief constructor
      /// @param out is the stream to which this data serializer
      ///        should push data.
      /// @param sPtr is the serialize object to use
      /// @note If sPtr is null, then an instance of BasicSerialize
      ///       will be used.
      Serializer(std::ostream         &out,
		 const Serialize::Ptr &sPtr=nullptr);
      
      /// @brief destrutor
      ~Serializer();

      /// @brief retrieve the associated serialie object
      /// @return the serialize object
      const Serialize::Ptr &GetSerialize() const;

      /// @brief Retreive the name
      /// @return name of Serialize object
      /// @note This string should match a paired Deserialize
      ///       object.
      const std::string &Name() const;

      /// @brief Retreive the version
      /// @return version of the object
      size_t Version() const;
      
      /// @brief Encode a boolean into an ostream.
      /// @param val is the value to encode
      void WriteBool  (bool val);

      /// @brief Encode an integer into an ostream.
      /// @param val is the value to encode
      void WriteInt(int val);

      /// @brief Encode a double into an ostream.
      /// @param val is the value to encode
      void WriteDouble(double val);

      /// @brief Encode a string into an ostream.
      /// @param data is the value to encode
      void WriteString(const std::string &data);

      /// @brief Encode a string into an ostream.
      /// @param data is a pointer to the beginning of a potentially
      ///        binary string.
      /// @param len is the lenght of data to encode as the string.
      void WriteString(const char *data, size_t len);

    private:
      std::ostream   &out;   ///< output
      Serialize::Ptr  sPtr;  ///< serialize implementation
    };

  }
  
}

#endif
