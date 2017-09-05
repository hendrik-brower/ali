#ifndef INCLUDED_ALI_SYSTEM_CODEC_DESERIALIZER
#define INCLUDED_ALI_SYSTEM_CODEC_DESERIALIZER

#include <aliSystem_codecDeserialize.hpp>
#include <string>
#include <iostream>

namespace aliSystem {
  namespace Codec {

    /// @brief Deserializer provides a set of extraction
    ///        API's for extracting common basic types from a
    ///        a stream.
    /// @note Since this object holds a reference to an istream
    ///       one should be careful with the life cycle of these
    ///       objects.  The life of the istream should exceed
    ///       the life of any referencing Deserializer.
    /// @note All functions assume the string is encoded
    ///       in a compatible format.  One should generally
    ///       verify or otherwise ensure that they were encoded
    ///       with a compatible Serializer.
    /// @note This class allows coding deserialization without
    ///       direct access to the underlying stream, which
    ///       helps ensure that the decoding is completely 
    ///       separated from the decoding of the content.
    struct Deserializer {

      /// @brief constructor
      /// @param in is the stream from which this data deserializer
      ///        should pull data.
      /// @param dPtr is the deserialize object to use
      /// @note If dPtr is null, then an instance of BasicDeserialize
      ///       will be used.
      Deserializer(std::istream           &in,
		   const Deserialize::Ptr &dPtr=nullptr);
      
      /// @brief destrutor
      ~Deserializer();

      /// @brief retrieve the associated deserialie object
      /// @return the deserialize object
      const Deserialize::Ptr &GetDeserialize() const;

      /// @brief Retreive the name
      /// @return name of the deserialize object
      /// @note This string should match a paired Serialize
      ///       object.
      const std::string &Name() const;

      /// @brief Retreive the version of the deserialize object
      /// @return version of the object
      size_t Version() const;

      /// @brief Return an indication as to whether the given
      ///        name/version can be deserialized with this
      ///        object.
      /// @param name is (presumably) the name of Serialize
      ///        object.
      /// @param version is (presumably) the verion of a
      ///        Serialize object
      /// @return true if this object can deserialzie a stream
      ///         encoded with a serializer with the noted
      ///         name/version.  Any serialize/deserialize
      ///         pair should follow the convention that
      ///         a serializer with a version<= a deserializer
      ///         should be handled.
      bool CanDeserialize(const std::string &name,
			  size_t version) const;

      /// @brief Return an indication of the istream's state.
      /// @return state indicator
      bool IsGood();

      /// @brief Return an indication of the istream's state.
      /// @return state indicator
      bool IsEOF();

      /// @brief Extract the remaining input.
      /// @param str a string to fill with the remaining input from
      ///        the stream
      void ReadAll(std::string &str);
      
      /// @brief Retrieve an indication of the next element in the
      ///        stream.
      /// @return Next type in the stream.
      /// @note One should be able to call this function multiple
      ///       times between calls to Read* without interfering
      ///       with the decoding of the input.
      Deserialize::Type NextType();
      
      /// @brief Extract a boolean from a istream.
      /// @return the extrated boolean
      /// @note If the stream does not contain a boolean
      ///       as the next element, this funnction will
      ///       throw an exception.
      bool ReadBool();
      
      /// @brief Extract a int from a istream.
      /// @return the extrated int
      /// @note If the stream does not contain a int
      ///       as the next element, this funnction will
      ///       throw an exception.
      int ReadInt();
      
      /// @brief Extract a double from a istream.
      /// @return the extrated double
      /// @note If the stream does not contain a double
      ///       as the next element, this funnction will
      ///       throw an exception.
      double ReadDouble();
      
      /// @brief Extract a string from a istream.
      /// @param data the extrated string
      /// @note If the stream does not contain a string
      ///       as the next element, this funnction will
      ///       throw an exception.
      std::string &ReadString(std::string &data);
      
    private:
      std::istream     &in;    ///< input stream
      Deserialize::Ptr  dPtr;  ///< pointer to deserialize implementation
    };

  }
}

#endif
