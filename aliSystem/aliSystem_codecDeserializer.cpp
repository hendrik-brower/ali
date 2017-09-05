#include <aliSystem_codecDeserializer.hpp>
#include <aliSystem_basicCodec.hpp>
#include <aliSystem_logging.hpp>

namespace aliSystem {
  namespace Codec {

    Deserializer::Deserializer(std::istream &in_,
			       const Deserialize::Ptr &dPtr_)
      : in(in_),
	dPtr(dPtr_) {
      dPtr = dPtr ? dPtr : BasicCodec::GetDeserializer();
      THROW_IF(!dPtr, "Attempt to construct a deserializer with an uninitialzed dPtr");
    }
      
    Deserializer::~Deserializer() {
    }

    const Deserialize::Ptr &Deserializer::GetDeserialize() const {
      return dPtr;
    }

    const std::string &Deserializer::Name() const {
      return dPtr->Name();
    }

    size_t Deserializer::Version() const {
      return dPtr->Version();
    }

    bool Deserializer::CanDeserialize(const std::string &name,
				      size_t version) const {
      return dPtr->CanDeserialize(name, version);
    }
    bool Deserializer::IsGood() {
      in.peek();
      return in.good();
    }

    bool Deserializer::IsEOF() {
      int v = in.peek();
      return v==EOF;
    }

    void Deserializer::ReadAll(std::string &remaining) {
      std::string str(std::istreambuf_iterator<char>(in), {});
      remaining = str;
    }

    Deserialize::Type Deserializer::NextType() {
      return dPtr->NextType(in);
    }
    
    bool Deserializer::ReadBool() {
      return dPtr->ReadBool(in);
    }
      
    int Deserializer::ReadInt() {
      return dPtr->ReadInt(in);
    }
      
    double Deserializer::ReadDouble() {
      return dPtr->ReadDouble(in);
    }
      
    std::string &Deserializer::ReadString(std::string &data) {
      return dPtr->ReadString(in, data);
    }

  }
}
