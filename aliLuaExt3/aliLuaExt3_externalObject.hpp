#ifndef INCLUDED_ALI_LUA_EXT3_EXTERNAL_OBJECT
#define INCLUDED_ALI_LUA_EXT3_EXTERNAL_OBJECT

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <memory>

namespace aliLuaExt3 {

  struct ExternalMT;
  /// @brief ExternalObject is used to represent the instantiation of
  ///        a Lua Object that maintains its own unique state.
  ///
  /// An ExternalObject can be passed around in c++ code and between
  /// Lua interpreters independenly of any Lua interpreter where it is
  /// created.
  ///
  /// Although the ExternalObject and/or ExternalMT does not explicitly
  /// support serialization/deserialization, it is concieveble that the
  /// state maintained by the object could be serialized and passed
  /// to another process or machine.
  ///
  /// The associated ExternalMT defines the script that is used to
  /// initialize an ExternalObject.  It also defines the top level
  /// functions that are defined by the object.  These top level
  /// functions should be scrpt functions within the script.  If
  /// they are not calls to them will fail.  If they are not declared
  /// as an exposed function in the ExternalMT, the call will be aborted
  /// thus providing restrictive access to the public funcitons defined
  /// within the interpreter's initialization script.
  struct ExternalObject {
    using Ptr           = std::shared_ptr<ExternalObject>;           ///< shared pointer
    using WPtr          = std::weak_ptr  <ExternalObject>;           ///< weak pointer
    using OBJ           = aliLuaCore::StaticObject<ExternalObject>;  ///< StaticObject
    using ExecPtr       = std::shared_ptr<aliLuaCore::Exec>;         ///< Exec Pointer
    using ExternalMTPtr = std::shared_ptr<ExternalMT>;               ///< ExternalMT pointer
    
    /// @brief Initialize ExternalObject module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Create provides a means to create an ExternalObject.
    /// @param mtPtr the ExternalMT defining the high level characteristics of the
    ///        ExternalObject to create.
    /// @param engineName is the name to use when creating an aliLuaCore::Exec
    ///        when creating the exec engine to hold the object's state.
    /// @param threadPool is the thread pool to use to create the aliLuaCore::Exec
    /// @param loadResult will be filled with any data returned when the script is loaded
    ///        into the newly created interpreter.
    /// @param createResult will be filled with any data returned by the create function,
    ///        which will be callsed after the interpreter is created and the script
    ///        loaded.
    /// @param args is a MakeFn defining any arguments to passs to the create function.
    /// @return an ExternalObject pointer
    /// @note If mtPtr is null, this function will through an exception.
    static Ptr Create(const ExternalMTPtr                   &mtPtr,
		      const std::string                     &engineName,
		      const aliSystem::Threading::Pool::Ptr &threadPool,
		      const aliLuaCore::Future::Ptr         &loadResult,
		      const aliLuaCore::Future::Ptr         &createResult,
		      const aliLuaCore::MakeFn              &args);

    /// @brief GetExec returns the exec that is owned by the ExternalObject.
    /// @return the pointer
    /// @note In C++, one has full access to the Exec and this allows by passing
    ///       and restrictions such as those imposed by the object's ExternalObject's
    ///       functions.  Care should be take to restrict interactions to only those
    ///       that are appropriate.
    const ExecPtr &GetExec () const;

    /// @brief EMTPtr will retrieve the object's ExternalMT.
    /// @return the ExternalMT.
    const ExternalMTPtr &EMTPtr() const;

    /// @brief RunFunction will run the specified function for the given ExternalObject.
    ///@param fnName is the name of the function to run
    /// @param future is the aliLuaCore::Future in which the call's result should be
    ///        placed.
    /// @param args is a MakeFn defining any arguments to pass to the function.
    void RunFunction(const std::string             &fnName,
		     const aliLuaCore::Future::Ptr &future,
		     const aliLuaCore::MakeFn      &args);

    /// @brief Destructor.
    ~ExternalObject();

    /// @brief Serialization function for ExternalObject objects.
    /// @param out is stream to which the object should be written
    /// @param o is the ExternalObject to serialize
    /// @return the stream passed as paramter out
    friend std::ostream &operator<<(std::ostream &out, const ExternalObject &o);
    
  private:

    /// @brief constructor
    ExternalObject();
   
    ExternalMTPtr emtPtr;  ///< ExternalMT pointer
    ExecPtr       ePtr;    ///< Exec pointer
  };
  
}

#endif
