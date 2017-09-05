#ifndef INCLUDED_ALI_LUA_EXT_EXEC_POOL
#define INCLUDED_ALI_LUA_EXT_EXEC_POOL

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <aliLuaExt_execPoolItem.hpp>
#include <deque>
#include <map>
#include <memory>
#include <set>

namespace aliLuaExt {

  struct ExecPoolInfo;

  /// @brief ExecPool provides an concrete implementation of an aliLuaCore::Exec.
  ///
  /// This class defines a set of non blocking Lua processor.  When constructed,
  /// the pool may create zero or more Exec's to execute within the pool.  When
  /// calls are passed to the ExecPool, they will execute on one of the member
  /// Exec.  In general, Exec's contained by a ExecPool object should be used as
  /// arbitrary processors where state is not retained between calls.  The Exec's
  /// will retain state, but two adjacently queued requests may not execute in the
  /// same Exec and as such they will not share Lua State.  Also, two squentially
  /// queued function calls may execute in an arbitrary order if the ExecPool
  /// contains more than one Exec.  To share state among a pool of Exec's, one
  /// can pass C++ objects that can be used to share state.  Be sure that these
  /// objects are safe for multithreaded access if this is done.
  ///
  /// The busy flag for this implementation is set the moment a call is registered
  /// through the Exec::Run function.  It will change to 'not-busy' when there
  /// is no currently executing Lua Calls and nothing is queued.
  ///
  /// OnIdle noficiations are sent when there is at least one Exec that is idle
  /// and has nothing in its queue.
  struct ExecPool : public aliLuaCore::Exec {
    using Ptr       = std::shared_ptr<ExecPool>; ///< shared pointer
    using WPtr      = std::weak_ptr<ExecPool>;   ///< weak pointer
    using ExecVec   = std::vector<Exec::WPtr>;   ///< vector of Exec
    using ExecMap   = std::map<const void*, aliLuaCore::Exec::Ptr>; ///< Map of Exec
    using ExecSet   = std::set<const void*>;  ///< Set of Exec addresses
    using LMap      = std::map<const void*, Listener::Ptr>; ///< map of Exec Listeners
    using ItemQueue = std::deque<ExecPoolItem::Ptr>;        ///< work queue
    using OBJ       = aliLuaCore::StaticObject<ExecPool>;   ///< Lua IO utility type
    using Pool      = aliSystem::Threading::Pool;           ///< a thread pool

    /// @brief Initialize ExecPoolInfo module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief Create will construct a ExecPool.
    /// @param name is the name of the pool. It does not need to be unique, though
    ///        troubleshooting will likly be easier if it is.
    /// @param numEngines is the initial number of member Exec's to create for the
    ///        pool.
    /// @param poolPtr is a thread pool which is used as the thread pool for any
    ///        member Exec's generated with  this call.
    /// @return An ExecPool pointer
    static Ptr Create(const std::string                     &name,
		      size_t                                 numEngines,
		      const aliSystem::Threading::Pool::Ptr &poolPtr);
    
    /// @brief GetInfo will return a make function that can be used to extract
    ///        details about the ExecPool and potentially other objects to which
    ///        it is related (eg member Exec).
    /// @param simple is a flag that indicates how much detail the constructed MakeFn
    ///        should include.  True (simple) will create less, false (detailed) more.
    /// @param ptr a pointer to a ExecPool.
    /// @return A make function that produces a table filled with information about the
    ///         ExecPool.
    /// @note If ptr does not hold a live pointer, this function will throw an exception.
    static aliLuaCore::MakeFn GetInfo(bool simple, const Ptr &ptr);
    
    /// @brief GetInfoDetailed will return a make function that can be used to extract
    ///        details about the ExecPool and potentially other objects to which
    ///        it is related (eg member Exec).
    /// @param ptr a pointer to a ExecPool.
    /// @return A make function that produces a table filled with information about the
    ///         ExecPool.
    /// @note If ptr does not hold a live pointer, this function will throw an exception.
    static aliLuaCore::MakeFn GetInfoDetailed(const Ptr &ptr);
    
    /// @brief GetInfoSimple will return a make function that can be used to extract
    ///        details about the ExecPool and potentially other objects to which
    ///        it is related (eg member Exec).
    /// @param ptr a pointer to a ExecPool.
    /// @return A make function that produces a table filled with information about the
    ///         ExecPool.
    /// @note If ptr does not hold a live pointer, this function will throw an exception.
    static aliLuaCore::MakeFn GetInfoSimple  (const Ptr &ptr);
    
    /// @brief Retrieve a shared pointer for a given instance.
    /// @return Exec of the instance on which this call is made
    Exec::Ptr GetExec() const override;
    
    /// @brief IsBusy returns true whenever work is either queued or in progress within
    ///        the ExecPool.
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
    
    /// @brief Register an Exec to participate within the ExecPool
    /// @param ePtr is the exec to add to the ExecPool
    /// @note If the ePtr is not dedicated to this ExecPool, its participation may
    ///       interfere with the timely execution of work pushed into this ExecPool.
    /// @note Registered Exec Engines should also implement the IsBusy and OnIdle
    ///       interfaces to ensure that the Exec can be effectively used without
    ///       interfering with the general operation of the ExecPool.
    void Register(const aliLuaCore::Exec::Ptr &ePtr);
    
    /// @brief Unregister an Exec from the Pool
    /// @param ePtr the Exec to unregister.
    void Unregister(const aliLuaCore::Exec::Ptr &ePtr);
    
    /// @brief Retrieve a list of the registered Exec.
    /// @param vec a vector to clear, then fill with the Registered Exec.
    /// @note One should be careful with Exec's registered to an ExecPool as
    ///       use outside of the ExecPool's interaces can disturb the general
    ///       behavior of the ExecPool.  In general these disturbances should not
    ///       be fatal.  However one should keep in mind the potential risks and
    ///       evaluate those risks effectively in their specific uses cases.
    void GetRegistered(ExecVec &vec);
    
    /// @brief NumPending returns the number of items pending in the ExecPool.
    /// @return the number of pending items.
    size_t NumPending();
    
    /// @brief destructor
    ~ExecPool();
    
    /// @brief ExecPool serialization operator
    /// @param out the stream to which the ExecPool should be serialized
    /// @param o the object to serialize
    /// @return the stream passed as out
    friend std::ostream &operator<<(std::ostream &out, const ExecPool &o);
    
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
    /// @brief Constructor
    /// @param name is the name of the ExecEngine
    ExecPool(const std::string &name);
    
    /// @brief Post is an internal function used to manage member Exec's operation
    /// @param exec is the address of a member exec.
    void Post(const void *exec);
    
    /// @brief SetIsBusy is an internal function used to latch and unlatch the
    ///        isBusy in a deterministic manner.
    Listeners::Ptr SetIsBusy(std::lock_guard<std::mutex> &g);
    
    WPtr           THIS;        ///< self pointer
    Pool::Ptr      poolPtr;     ///< threading pool
    bool           isBusy;      ///< isBusy flag
    std::mutex     lock;        ///< lock for managing member values in a thread safe manner
    ExecMap        execMap;      ///< map of member Exec
    ExecSet        idle;         ///< map of idle Exec members
    Listeners::Ptr onIdle;       ///< idle listener container
    ItemQueue      itemQueue;    ///< work queue
    LMap           listenerMap;  ///< map of Exec listeners
    friend struct ExecPoolInfo;
  };
  
}

#endif

