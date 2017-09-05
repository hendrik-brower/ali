#include <aliSystem.hpp>

namespace aliSystem {

  void RegisterInitFini(ComponentRegistry &cr) {
    Component::Ptr ptr = cr.Register("aliSystem");
    ptr->AddDependency("aliSystem::BasicCodec");
    ptr->AddDependency("aliSystem::Hold");
    ptr->AddDependency("aliSystem::Threading::Scheduler");
    
    BasicCodec          ::RegisterInitFini(cr);
    Hold                ::RegisterInitFini(cr);
    Threading::Scheduler::RegisterInitFini(cr);
  }

}
