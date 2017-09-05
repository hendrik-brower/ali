#ifndef INCLUDED_ALI_LUA_CORE_EXEC
#define INCLUDED_ALI_LUA_CORE_EXEC

#include <aliLuaCore_functions.hpp>
#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_staticObject.hpp>
#include <aliLuaCore_types.hpp>
#include <aliSystem.hpp>
#include <functional>
#include <memory>
#include <string>

namespace aliLuaCore {

  struct Future;

  /// @brief Exec defines the basic wrapper around a Lua interpreter.
  ///
  /// Exec is an abstract class designed to support (favor) asynchronous
  /// execution of scripting logic.
  struct Exec {
    using Ptr       = std::shared_ptr<Exec>;               ///< shared pointer
    using WPtr      = std::weak_ptr<Exec>;                 ///< weak pointer
    using OBJ       = StaticObject<Exec>;                  ///< object accessor
    using FuturePtr = std::shared_ptr<Future>;             ///< future
    using Listener  = aliSystem::Listener<const WPtr &>;   ///< exec listener
    using Listeners = aliSystem::Listeners<const WPtr &>;  ///< exec listener collection

    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief Retrieve the exec type
    /// @return the execution type
    const std::string               &GetExecType() const;
    
    /// @brief Retrieve the exec's name
    /// @return name
    const std::string               &Name() const;
    
    /// @brief Retrieve the exec's stats
    /// @return stats poniter
    const aliSystem::Stats::Ptr     &Stats() const;
    
    /// @brief Retrieev the exec's registry
    /// @return registry
    /// @note The registry's primary purpose is to hold
    ///       pointers that should generally be disposed
    ///       when the Exec is destructed.
    /// @note The Registry interface supports more explicit
    ///       management of life cycle elements if desired or
    ///       necessary.
    /// @note A common use pattern would be places where a
    ///       light user data pointer is pushed into Lua.
    /// @note Another common use cases is when pushing references
    ///       as upvalues.  It wsa observed objects with metatables
    ///       pushed as up values do not necessarily get their
    ///       gc routine called.  This may be an issue specific
    ///       to a specific version(s) of Lua, it might also
    ///       be an inaccurate observation.
    const aliSystem::Registry::Ptr  &Registry() const;
    
    /// @brief Retriee information about the exec object.
    /// @return Exec's info table
    MakeFn GetInfo();
    
    /// @brief Assign an info table key value.
    /// @param key for data
    /// @param makeFn a function to construct the info.
    /// @note The makeFn should produce exactly 1 value.
    ///       If it does not, calls to load the MakeFn returned
    ///       By GetInfo may throw an exception.
    void SetInfo(const std::string &key, const MakeFn &makeFn);
    
    /// @brief SetStats allows reassigning the stats object
    ///        associated with the exec.
    /// @param sPtr new stats object (may be null)
    void SetStats(const aliSystem::Stats::Ptr &sPtr);
    
    /// @brief Run a Lua function
    /// @param luaFn is the function to run
    /// @note One should generally assume that the passed function
    ///       will be run in the background on a separate thread
    ///       after this call returns.
    void Run(const LuaFn &luaFn);
    
    /// @brief Run a Lua function, retrieving any returned values
    ///        in a future.
    /// @param future to catch any returned valued (or error)
    /// @param luaFn the function to run.
    /// @note One should generally assume that the passed function
    ///       will be run in the background on a separate thread
    ///       after this call returns.
    void Run(const FuturePtr &future, const LuaFn &luaFn);
    
    /// @brief Retrieve a shared pointer to the given exec.
    /// @return a shared pointer
    /// @note The return value may be null.  It is dependent
    ///       on the derived class's implementation.
    virtual Ptr GetExec() const = 0;
    
    /// @brief IsBusy will return true if the Exec is busy.
    /// @return busy flag
    /// @note The definition may have subtle interpretation differences
    ///       between derived class implemenations.  In general, it is
    ///       advised to return true when the object is going to queue
    ///       work.  However,  It could return true if the derived class
    ///       is less than 100% busy.  To be less than 100% busy, but
    ///       greater than 0%, the derived class would likey wrap
    ///       more than one interpreter.
    virtual bool IsBusy() const = 0;
    
    /// @brief Retrieve the OnIdle listeners collection.
    /// @return the listeners object.
    virtual const Listeners::Ptr &OnIdle() const = 0;
    
    /// @brief Destructor
    virtual ~Exec();
    
    /// @brief Serialization function for Exec objects.
    /// @param out is stream to which the object should be written
    /// @param o is the exec to serialize
    /// @return the stream passed as paramter out
    friend std::ostream &operator<<(std::ostream &out, const Exec &o);
    
  protected:
    /// @brief constructor
    /// @param execType a string that describes the exec type.
    ///        Generally this would be constant for a given
    ///        type of derived object, though it this is not
    ///        a requirement.
    /// @param name the name of the Exec.
    Exec(const std::string &execType, const std::string &name);
    
    /// @brief a virtual run function.
    /// @param future where any error or returned values
    ///        should be captured.
    /// @param luaFn the function to run
    /// @note This function should support recursive calls.
    /// @note This function may queue work for later execution.
    /// @note If a derived class wraps more than one interpreter,
    ///       it is up to the class designer to what they do
    ///       within this API.
    virtual void InternalRun(const FuturePtr   &future,
			     const LuaFn       &luaFn) = 0;
    
  private:
    std::string              execType;  ///< exec type
    std::string              name;      ///< name
    aliSystem::Stats::Ptr    statsPtr;  ///< stats
    aliSystem::Registry::Ptr registry;  ///< registry
    MakeTableUtil            info;      ///< info MTU to support GetInfo calls
  };
  
}

#endif
