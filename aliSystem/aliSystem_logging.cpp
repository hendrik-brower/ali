#include <aliSystem_logging.hpp>

namespace aliSystem {
  namespace Logging {
    
    bool IsDebug() { return false; }

    std::string CurTime() {
      std::ostringstream ss;
      ss << aliSystem::Time::Now();
      return ss.str();
    }
  }
}

