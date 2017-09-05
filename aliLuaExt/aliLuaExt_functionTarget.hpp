#ifndef INCLUDED_ALI_LUA_EXT_FUNCTION_TARGET
#define INCLUDED_ALI_LUA_EXT_FUNCTION_TARGET

#include <aliLuaCore.hpp>
#include <memory>
#include <string>

namespace aliLuaExt {

  /// @brief FunctionTarget defines a object that extends
  ///        the abstract CallTarget to support function
  ///        based call target.
  ///
  /// The call target can be passed across interpreters
  /// and will properly run as long as the interpreter
  /// in which it is run defines the function specified
  /// by this call.
  struct FunctionTarget : public aliLuaCore::CallTarget {
    using Ptr = std::shared_ptr<FunctionTarget>; ///< shared ptr

    /// @brief Initialize FunctionTarget module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Constructor
    /// @param fnName is the name of the function to
    ///        be called when the function target is run.
    /// @note The fnName needs to be a global value; however,
    ///       the string fnName merely needs to accurately
    ///       describe the function name so subtable or index
    ///       elements can be included in the function nae.
    ///       Thus, the following are valid fnNames: lib.xyz.FnY
    ///       or lib.xyz.fnTable[4].
    FunctionTarget(const std::string &fnName);
    
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
    void Run(const aliLuaCore::Exec::Ptr &ePtr,
	     const aliLuaCore::Future::Ptr &fPtr,
	     const aliLuaCore::MakeFn &args) override;
    
    /// @brief FnName returns the fnName defined by this call target.
    /// @return the function name
    const std::string &FnName() const;

    /// @brief GetInfo exposes a function that will
    ///        extract information about the Function target.
    /// @return a MakeFn that defines a table of information.
    aliLuaCore::MakeFn GetInfo();
    
  private:
    std::string fnName; ///< stored fn name
  };

}


#endif
