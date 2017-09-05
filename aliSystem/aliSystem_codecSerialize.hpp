#ifndef INCLUDED_ALI_SYSTEM_CODEC_SERIALIZE
#define INCLUDED_ALI_SYSTEM_CODEC_SERIALIZE

#include <memory>
#include <string>
#include <iostream>

namespace aliSystem {
  namespace Codec {

    /// @brief Serialize provides a set of serialization
    ///        API's for encoding common basic types to a
    ///        a stream.
    struct Serialize {

      using Ptr = std::shared_ptr<Serialize>;    ///< shared pointer

      /// @brief constructor
      /// @param name is the name of the object
      /// @param version is the version of the object
      Serialize(const std::string &name,
		size_t version);
      
      /// @brief destrutor
      virtual ~Serialize();

      /// @brief Retreive the name
      /// @return name of Serialize object
      /// @note This string should match a paired Deserialize
      ///       object.
      const std::string &Name() const;

      /// @brief Retreive the version
      /// @return version of the object
      size_t Version() const;
      
      /// @brief Encode a boolean into an ostream.
      /// @param out - stream to which the value should be
      ///        encoded
      /// @param val is the value to encode
      virtual void WriteBool  (std::ostream &out, bool val) = 0;

      /// @brief Encode an integer into an ostream.
      /// @param out - stream to which the value should be
      ///        encoded
      /// @param val is the value to encode
      virtual void WriteInt(std::ostream &out, int val) = 0;

      /// @brief Encode a double into an ostream.
      /// @param out - stream to which the value should be
      ///        encoded
      /// @param val is the value to encode
      virtual void WriteDouble(std::ostream &out, double val) = 0;

      /// @brief Encode a string into an ostream.
      /// @param out - stream to which the value should be
      ///        encoded
      /// @param data is the value to encode
      virtual void WriteString(std::ostream &out, const std::string &data) = 0;

      /// @brief Encode a string into an ostream.
      /// @param out - stream to which the value should be
      ///        encoded
      /// @param data is a pointer to the beginning of a potentially
      ///        binary string.
      /// @param len is the lenght of data to encode as the string.
      virtual void WriteString(std::ostream &out, const char *data, size_t len) = 0;

    private:
      std::string name;     ///< name
      size_t      version;  ///< version
    };

  }
  
}

#endif
