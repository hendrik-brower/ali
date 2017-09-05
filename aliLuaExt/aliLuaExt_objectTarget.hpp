#ifndef aliLINCLUDED_ALI_LUA_EXT_OBJECT_TARGET
#define aliLINCLUDED_ALI_LUA_EXT_OBJECT_TARGET

#include <aliLuaCore.hpp>
#include <string>

namespace aliLuaExt {

  /// @brief ObjectTarget defines a object that extends
  ///        the abstract CallTarget to support an object
  ///        based call target.
  ///
  /// The call target can be passed across interpreters
  /// and will properly run as long as the interpreter
  /// in which it is run defines the object specified
  /// by this call.
  struct ObjectTarget : public aliLuaCore::CallTarget {
    using Ptr = std::shared_ptr<ObjectTarget>; ///< shared ptr
    
    /// @brief Initialize ObjectTarget module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Constructor
    /// @param fnName is the name of the metatable function to
    ///        be called when the function target is run.
    /// @param mtPtr is the aliLuaCore::MT type for the object
    ///        that one wishes to target with the new instance.
    /// @param object is a MakeFn that will construct the target
    ///        object.
    ObjectTarget(const std::string         &fnName,
		 const aliLuaCore::MT::Ptr &mtPtr,
		 const aliLuaCore::MakeFn  &object);
    
    /// @brief Run the function defined by this target.
    /// @param ePtr is a pointer to the aliLuaCore::Exec in which
    ///        the function should be run.
    /// @param fPtr a future into which the result of the function
    ///        call should be placed.
    /// @param args is a MakeFn that extracts 0 or more arguments
    ///        which will be passed as parameters.
    /// @note If ePtr is null, this function will throw an exception.
    /// @note If fPtr is null, no results will be recorded;
    ///       however, the call will run without throwing an
    ///       exception.
    void Run(const aliLuaCore::Exec::Ptr   &ePtr,
	     const aliLuaCore::Future::Ptr &fPtr,
	     const aliLuaCore::MakeFn      &args) override;
    
    /// @brief FnName returns the fnName defined by this call target.
    /// @return the function name
    const std::string &FnName   () const;
    
    /// @brief GetMT returns the MT associated with the given
    ///        ObjectTarget.
    /// @return mt object
    const aliLuaCore::MT::Ptr     &GetMT    () const;
    
    /// @brief GetObject returns the MakeFn that will construct the object
    /// @return object make function
    const aliLuaCore::MakeFn      &GetObject() const;
    
    /// @brief GetInfo exposes a function that will
    ///        extract information about the Object target.
    /// @return a table of information
    aliLuaCore::MakeFn GetInfo();
    
  private:
    std::string         fnName;    ///< object's metatable function name
    aliLuaCore::MT::Ptr mtPtr;     ///< object's metatable
    aliLuaCore::MakeFn  object;    ///< object
  };

}

#endif
