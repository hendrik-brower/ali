#include <aliLuaExt3.hpp>

namespace aliLuaExt3 {

  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt3");
    ptr->AddDependency("aliLuaCore");
    ptr->AddDependency("aliLuaExt3::ExternalMT");
    ptr->AddDependency("aliLuaExt3::ExternalObject");
    ExternalMT    ::RegisterInitFini(cr);
    ExternalObject::RegisterInitFini(cr);
  }
  
}
