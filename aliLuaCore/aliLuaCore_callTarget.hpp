#ifndef INCLUDED_ALI_LUA_CORE_CALL_TARGET
#define INCLUDED_ALI_LUA_CORE_CALL_TARGET

#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_staticObject.hpp>
#include <aliLuaCore_types.hpp>
#include <memory>

struct lua_State;
namespace aliLuaCore {
  
  struct Exec;
  struct Future;
  struct InfoMap;
  /// @brief CallTarget wraps a function or object pointer such that
  ///        the wrapped object can be run with arbitrary arguments.
  ///
  /// The CallTarget is an abstrat base class that can be derived
  /// to define the specifics of the execution logic.  These derived
  /// classes are registered with a tagetType, which is used as
  /// the first argument to the Lua Create function to select which
  /// derived class createFn to call.
  struct CallTarget {
    using Ptr       = std::shared_ptr<CallTarget>;  ///< pointer to instance objects
    using ExecPtr   = std::shared_ptr<Exec>;        ///< Forward declaration of Exec::Ptr
    using FuturePtr = std::shared_ptr<Future>;      ///< Forward declearation of a Future::Ptr
    using OBJ       = StaticObject<CallTarget>;     ///< A public OBJ interface
    using CreateFn  = std::function<Ptr (lua_State *L)>; ///< A createFn handler
    
    /// @brief Initialize module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief construtor
    CallTarget();
    
    /// @brief destructor
    virtual ~CallTarget();
    
    /// @brief Run provides a means of running an instance of a CallTarget.
    /// @param ePtr the Exec::Ptr in which to run the function
    /// @param args are the arguments to pass to the function
    /// @note The actual arguments sent to the function may be manipulated
    ///       in the implemenation of a derived class.  For example, an aliLuaCore::ObjectTarget
    ///       wraps the object to call as well as the function, so the argumetns do not
    ///       need to include the object on which to work.
    /// @note This function is a pass through to the derived class's Run w/Future.
    void Run(const ExecPtr &ePtr, const MakeFn &args);
    
    /// @brief Run provides a means of running an instance of a CallTarget.
    /// @param ePtr the Exec::Ptr in which to run the function
    /// @param fPtr a future to store any data returned from the call
    /// @param args are the arguments to pass to the function
    /// @note The actual arguments sent to the function may be manipulated
    ///       in the implemenation of a derived class.  For example, an aliLuaCore::ObjectTarget
    ///       wraps the object to call as well as the function, so the argumetns do not
    ///       need to include the object on which to work.
    virtual void Run(const ExecPtr &ePtr, const FuturePtr &fPtr, const MakeFn &args) = 0;
    
    /// @brief GetInfo will return the object's information
    /// @return a MakeFn producing a table of info.
    virtual MakeFn GetInfo() = 0;
    
    /// @brief Register allows registering a create function for a given type of CallTarget.
    /// @param targetType type name of the call target.  This value must be globally unique.
    /// @param createFn is a function handler that will be called when ever the "call target
    ///        create fn" with the specified targetType.  The first argument (an options table) 
    ///        is a table which should generally be used to define the arguments for the call 
    ///        target.Additional arguments my be processed if appropriate, though for consistency,
    ///        it is preferable to extract arguments from the options table.
    static void Register(const std::string &targetType, const CreateFn &createFn);
    
    /// @brief Unregister a call target.
    /// @param targetType name to unregister
    /// @note Once a call target is unregistered, any subsequent calls will fail with an
    ///       error indicating the call target type is unrecognized.
    static void Unregister(const std::string &targetType);
    
  };

}

#endif
