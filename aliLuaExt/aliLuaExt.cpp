#include <aliLuaExt.hpp>
#include <aliSystem.hpp>

namespace aliLuaExt {

  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt");
    ptr->AddDependency("aliLuaCore");
    ptr->AddDependency("aliLuaExt::Codec");
    ptr->AddDependency("aliLuaExt::ExecEngine");
    ptr->AddDependency("aliLuaExt::ExecPool");
    ptr->AddDependency("aliLuaExt::FunctionTarget");
    ptr->AddDependency("aliLuaExt::Hold");
    ptr->AddDependency("aliLuaExt::IO");
    ptr->AddDependency("aliLuaExt::ObjectTarget");
    ptr->AddDependency("aliLuaExt::Queue");
    ptr->AddDependency("aliLuaExt::Threading");
    
    Codec         ::RegisterInitFini(cr);
    ExecEngine    ::RegisterInitFini(cr);
    ExecPool      ::RegisterInitFini(cr);
    FunctionTarget::RegisterInitFini(cr);
    Hold          ::RegisterInitFini(cr);
    IO            ::RegisterInitFini(cr);
    ObjectTarget  ::RegisterInitFini(cr);
    Queue         ::RegisterInitFini(cr);
    Threading     ::RegisterInitFini(cr);
  }
  
}
