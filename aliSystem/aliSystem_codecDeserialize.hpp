#ifndef INCLUDED_ALI_SYSTEM_CODEC_DESERIALIZE
#define INCLUDED_ALI_SYSTEM_CODEC_DESERIALIZE

#include <memory>
#include <string>
#include <iostream>

namespace aliSystem {
  namespace Codec {

    /// @brief Deserialize provides a set of extraction
    ///        API's for extracting common basic types from a
    ///        a stream.
    /// @note All functions assume the string is encoded
    ///       in a compatible format.  On should generally
    ///       assume that they were encoded with the
    ///       peer Serialize.
    struct Deserialize {
      
      using Ptr = std::shared_ptr<Deserialize>;  ///< shared pointer

      /// @brief Type defines a set of types that define the values
      ///        that may exist within the stream.
      /// @note END is indications of the streams
      ///       state rather than a value within the stream.
      /// @note INVALID is used to indicate a condition where input
      ///       is corrupt and that the stream includes unexpected data.
      enum class Type { END, INVALID, BOOL, INT, DOUBLE, STRING };

      /// @brief constructor
      /// @param name is the name of the object
      /// @param version is the version of the object
      Deserialize(const std::string &name,
		  size_t version);
      
      /// @brief destrutor
      virtual ~Deserialize();

      /// @brief Retreive the name
      /// @return name of the object
      /// @note This string should match a paired Serialize
      ///       object.
      const std::string &Name() const;

      /// @brief Retreive the version
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

      /// @brief Retrieve the type of the next value in
      ///        the stream.
      /// @param in is the stream to check.
      /// @return the next item's type
      /// @note Impelmentations should ensure that they
      ///       do not index the stream so that this call
      ///       may be made many times without fear of
      ///       consuming data from the stream.
      virtual Type NextType(std::istream &in) = 0;

      /// @brief Extract a boolean from a istream.
      /// @param in - stream from which the value should be
      ///        extracted.
      /// @return the extrated boolean
      /// @note If the stream does not contain a boolean
      ///       as the next element, this funnction will
      ///       throw an exception.
      virtual bool ReadBool(std::istream &in) = 0;
      
      /// @brief Extract a int from a istream.
      /// @param in - stream from which the value should be
      ///        extracted.
      /// @return the extrated int
      /// @note If the stream does not contain a int
      ///       as the next element, this funnction will
      ///       throw an exception.
      virtual int ReadInt(std::istream &in) = 0;
      
      /// @brief Extract a double from a istream.
      /// @param in - stream from which the value should be
      ///        extracted.
      /// @return the extrated double
      /// @note If the stream does not contain a double
      ///       as the next element, this funnction will
      ///       throw an exception.
      virtual double ReadDouble(std::istream &in) = 0;
      
      /// @brief Extract a string from a istream.
      /// @param in - stream from which the value should be
      ///        extracted.
      /// @param data the extrated string
      /// @note If the stream does not contain a string
      ///       as the next element, this funnction will
      ///       throw an exception.
      virtual std::string &ReadString(std::istream &in, std::string &data) = 0;
      
    private:
      std::string name;     ///< name
      size_t      version;  ///< version
    };

  }
}

#endif
