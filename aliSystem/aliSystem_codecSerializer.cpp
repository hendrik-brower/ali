#include <aliSystem_codecSerializer.hpp>
#include <aliSystem_basicCodec.hpp>
#include <aliSystem_logging.hpp>

namespace aliSystem {
  namespace Codec {

    Serializer::Serializer(std::ostream &out_,
			   const Serialize::Ptr &sPtr_)
      : out(out_),
	sPtr(sPtr_) {
      sPtr = sPtr ? sPtr : BasicCodec::GetSerializer();
      THROW_IF(!sPtr, "sPtr is uninitialized");
    }
      
    Serializer::~Serializer() {
    }

    const Serialize::Ptr &Serializer::GetSerialize() const {
      return sPtr;
    }

    const std::string &Serializer::Name() const {
      return sPtr->Name();
    }

    size_t Serializer::Version() const {
      return sPtr->Version();
    }
      
    void Serializer::WriteBool  (bool val) {
      sPtr->WriteBool(out, val);
    }

    void Serializer::WriteInt(int val) {
      sPtr->WriteInt(out, val);
    }

    void Serializer::WriteDouble(double val) {
      sPtr->WriteDouble(out, val);
    }

    void Serializer::WriteString(const std::string &data) {
      sPtr->WriteString(out, data);
    }

    void Serializer::WriteString(const char *data, size_t len) {
      sPtr->WriteString(out, data, len);
    }

  }
}
