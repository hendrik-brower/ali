#include <aliLuaExt2.hpp>
#include <aliSystem.hpp>


namespace aliLuaExt2 {

  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt2");
    ptr->AddDependency("aliLuaCore");
    ptr->AddDependency("aliLuaExt");
    ptr->AddDependency("aliLuaExt2::Action");
    
    Action::RegisterInitFini(cr);
  }
  
}
