#include <aliLuaCore.hpp>

namespace aliLuaCore {

  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore");
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::CallTarget");
    ptr->AddDependency("aliLuaCore::Exec");
    ptr->AddDependency("aliLuaCore::Future");
    ptr->AddDependency("aliLuaCore::Module");
    ptr->AddDependency("aliLuaCore::MT");
    ptr->AddDependency("aliLuaCore::Stats");
    CallTarget::RegisterInitFini(cr);
    Exec      ::RegisterInitFini(cr);
    Future    ::RegisterInitFini(cr);
    Module    ::RegisterInitFini(cr);
    MT        ::RegisterInitFini(cr);
    Stats     ::RegisterInitFini(cr);
  }
  
}

