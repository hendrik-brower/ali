#include <aliSystem_codecSerialize.hpp>
#include <aliSystem_logging.hpp>

namespace aliSystem {
  namespace Codec {

    Serialize::Serialize(const std::string &name_,
			 size_t version_)
      : name(name_),
	version(version_) {
    }
    Serialize::~Serialize() {
    }
    const std::string &Serialize::Name() const {
      return name;
    }
    size_t Serialize::Version() const {
      return version;
    }

  }
}
