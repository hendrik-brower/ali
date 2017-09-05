#ifndef INCLUDED_ALI_LUA_CORE_MODULE
#define INCLUDED_ALI_LUA_CORE_MODULE

#include <aliSystem.hpp>
#include <functional>
#include <memory>
#include <string>

namespace aliLuaCore {
  struct Exec;

  /// @brief The module interface is intended to define module related
  ///        handlers.
  ///
  ///        A module refers to something that is initialized
  ///        into a Lua engine.
  struct Module {
    using ExecPtr = std::shared_ptr<Exec>;  ///< forward declaration
    using ExecFn  = std::function<void(const ExecPtr &)>; ///< Module handler
    
    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Register allows one to register some functionality to run
    ///        when an Exec is created.
    ///
    ///        The functions will be queued to run in the Engine creation
    ///        function.  In general, one should not manually run the
    ///        Module::InitEngine.
    /// @param name is a description of the registered function.
    /// @param execFn is a function to schedule to run when an engine is
    ///        first created.
    /// @note If multipe functions are registered, they will be run in the
    ///       order in which they were initialized.
    /// @note Any values wrapped by the execFn will be held until the
    ///       module is shutdown.
    static void Register(const std::string &name, ExecFn execFn);

    /// @brief When InitEngine is called, all currently registered modules
    ///        will be run in the passed engine.
    /// @param ePtr is the engien to run the registered module functions.
    /// @note Generally, the functions will be queued for execution and
    ///       these items will execute in the order queued, thought that is
    ///       dependent upon aspects of the Exec.  If they are not run
    ///       in order and before other functions are queued, that should
    ///       be considered a critical bug.
    /// @note This function should really only be called in the Exec's
    ///       Create function or some other Exec that owns (defines and
    ///       creates a lua_State).  Repeating the quueing of functions
    ///       will introduce undefined behaviors.
    static void InitEngine(const ExecPtr &ePtr);
    
  };
  
}

#endif
