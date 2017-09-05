#include <aliSystem_codecDeserialize.hpp>
#include <aliSystem_logging.hpp>

namespace aliSystem {
  namespace Codec {

    Deserialize::Deserialize(const std::string &name_,
			     size_t version_)
      : name(name_),
	version(version_) {
    }
    Deserialize::~Deserialize() {
    }
    const std::string &Deserialize::Name() const {
      return name;
    }
    size_t Deserialize::Version() const {
      return version;
    }
    bool Deserialize::CanDeserialize(const std::string &name,
				     size_t             version) const {
      return name==Name() && version<=Version();
    }

  }
}
