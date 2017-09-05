#ifndef INCLUDED_ALI_LUA_EXT_EXEC_ENGINE
#define INCLUDED_ALI_LUA_EXT_EXEC_ENGINE

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct lua_State;
namespace aliLuaExt {

  /// @brief ExecEngine provides an concrete implementation of an aliLuaCore::Exec
  ///
  /// This class defines a non blocking Lua processor.  When constructed,
  /// all registered modules are loaded into the interpreter in the order that
  /// the were registered to aliLuaCore::Module.
  ///
  /// The busy flagg for this implementation is set the moment a call is registered
  /// through the Exec::Run function.  If the thread pool is busy with other
  /// work, the actual work may be delayed, thus IsBusy will reflect a state
  /// where work is queued and/or executing.
  ///
  /// OnIdle noficiations are sent when work completes and there is nothing in
  /// the queue.
  struct ExecEngine : public aliLuaCore::Exec {
    using Ptr       = std::shared_ptr<ExecEngine>;  ///< shared pointer
    using WPtr      = std::weak_ptr<ExecEngine>;    ///< weak pointer
    using OBJ       = aliLuaCore::StaticObject<ExecEngine>; ///< Static OBJ utility type
    using Pool      = aliSystem::Threading::Pool;           ///< Thread pool
    using Queue     = aliSystem::Threading::Queue;              ///< Thread pool's queue
    using QListener = aliSystem::Listener<const Queue::WPtr&>;  ///< Listener
    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);

    /// @brief Create will construct a ExecEngine.
    /// @param name is the name of the engine. It does not need to be unique, though
    ///        troubleshooting will likly be easier if it is.
    /// @param pool is a thread pool through which work should be executed.
    /// @return An ExecEngine pointer
    static Ptr Create(const std::string &name, const Pool::Ptr &pool);
    
    /// @brief Retrieve the ExecEngine for an arbitrary Lua interpreter.
    /// @param L the Lua state from which the ExecEngine is sought.
    /// @return The Lua State's ExecEngine (or null)
    /// @note If the Lua interpreter is encapsulated by an instance of this
    ///       object, this function shuold return a null pointer.
    /// @note It is possible to configure the interperter such that this call
    ///       will return an ExecEngine (that does not own the interpreter).
    ///       It is probably unwise to do so.
    static Ptr GetEngine(lua_State *L);
    
    /// @brief GetInfo will return a make function that can be used to extract
    ///        details about the ExecEngine and potentially other objects to which
    ///        it is related.
    /// @param ptr a pointer to a Lua engine.
    /// @return A make function that produces a table filled with information about the
    ///         ExecEngine.
    /// @note If ptr does not hold a live pointer, then this fuction will throw an
    ///       exception.
    static aliLuaCore::MakeFn GetInfo(const Ptr &ptr);
    
    /// @brief Retrieve a shared pointer for a given instance.
    /// @return Exec of the instance on which this call is made
    aliLuaCore::Exec::Ptr  GetExec() const override;
    
    /// @brief IsBusy returns true whenever work is either queued or in progress within
    ///        the ExecEngine.
    /// @return busy flag.
    bool IsBusy() const override;
    
    /// @brief Returns the instance's OnIdle listener container.
    /// @return The listener object.
    /// @note External code may use this to receive messages whenever the given instance
    ///       becomes idle.
    /// @note If it is critical to only act when the instance is idle, one might be
    ///       cautious of how OnIdle messages are processed.  Since the Listener container
    ///       is public, its possble for external code to trigger OnIdle messages at
    ///       arbitrary points in time.
    const Listeners::Ptr &OnIdle() const override;
    
    /// @brief destructor
    ~ExecEngine();
    
    /// @brief a serialization function for ExecEngine
    /// @param out the stream to serialize the engine
    /// @param o the engine object to serialize
    /// @return the stream passed as out
    friend std::ostream &operator<<(std::ostream &out, const ExecEngine &o);
    
  protected:
    /// @brief Internal run function.  This is called from aliLuaCore::Exec's Run
    ///        function.
    /// @param future is a container for the results (if any) from the given Lua call.
    /// @param luaFn is the function to run.  Note that the luaFn is a functor and
    ///        may be a C++ object that wraps additional data with the functor.
    /// @note When this function returns, the passed function may have not been executed.
    ///       Instead it may sit in a queue until a later time, or if the instance is
    ///       destroyed, it may never be executed.
    void InternalRun(const aliLuaCore::Future::Ptr &future,
		     const aliLuaCore::LuaFn       &luaFn) override;
    
  private:
    /// @brief PrivateRun is an internal helper function that is called when a
    ///        queue function will actually be executed.
    /// @param ePtr is the pointer to the ExecEngine on which a funciton should be run
    /// @param future is the object that will be loaded with the results of the
    ///        function call.
    /// @param luaFn is the functor that will be executed.
    static void PrivateRun(const Ptr                     &ePtr,
			   const aliLuaCore::Future::Ptr &future,
			   const aliLuaCore::LuaFn       &luaFn);
    
    /// @brief Constructor
    /// @param name is the name of the ExecEngine
    ExecEngine(const std::string &name);
    
    WPtr                 THIS;          ///< internal "self" pointer
    std::recursive_mutex runLock;        ///< mutex to prevent concurrent access to L
    lua_State           *L;              ///< Lua State
    Queue::Ptr           queue;          ///< work queue
    bool                 isBusy;         ///< busy flag
    Listeners::Ptr       onIdle;         ///< on idle listener container
    QListener::Ptr       queueListener;  ///< An idle listener for the instance's thread queue
  };

}

#endif
