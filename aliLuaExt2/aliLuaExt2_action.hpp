#ifndef INCLUDED_ALI_LUA_EXT2_ACTION
#define INCLUDED_ALI_LUA_EXT2_ACTION

#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <unordered_set>

namespace aliLuaExt2 {

  /// @brief An Action is a high level object that supports
  ///        defining logic to execute based on some triggering
  ///        criteria or a timeout.
  ///
  /// The object supports distributed execution and triggers
  /// only if the associated target and triggers support that
  /// ability.
  ///
  /// Though it is not supported at this time, it may be possible
  /// to support serialization of the Action, which if done in
  /// a way that supports distributed triggers and targets, then
  /// an Action might be something that could be transmitted to
  /// an Action server for managing the execution.  Alternatively
  /// it would be an object that could be transferred to another
  /// process in the event that the original process is shutdown.
  ///
  /// Overall, the Action repreents a slight opening of the door
  /// to some relatively advanced capabilities.
  ///
  /// In many ways this class is a trival implemetation.  For
  /// example, the triggers are merely a set of futures that
  /// all must be set (and not errored) for the action to trigger.
  /// It would be much more friendly if an expression could be
  /// defiend that was composed of and, or, not,  and relational
  /// operators as well as boolean flags, strings, and scalar
  /// values.  An opportunity for the future.
  struct Action {
    using Ptr    = std::shared_ptr<Action>;              ///< shared pointer
    using WPtr   = std::weak_ptr  <Action>;              ///< weak pointer
    using OBJ    = aliLuaCore::StaticObject<Action>;     ///< static object
    using DepVec = std::vector<aliLuaCore::Future::Ptr>; ///< future vector

    /// @brief a future listener - Or in the context of this class, a
    ///        dependecny function.
    using DepFn  = std::function<void(const aliLuaCore::Future::Ptr &)>;
    
    /// @brief Initialize Action module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaExt::RegisterInitFini
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief Create an Action object
    /// @param name is the name of the action, it does not need to be
    ///        unique.
    /// @param target is a target to call when the dependencies are met
    /// @param exec is the Exec in which the target should be executed
    /// @param args is Arguments to pass the call target when the action
    ///        executes.
    /// @param utcTimeout is a absolute time that will "disable" the
    ///        action with a timeout error.
    /// @param dependencies are a list of futures that when all have
    ///        triggered, the action will execute (if it hasn't already
    ///        timed out.
    /// @return A pointer to a new Action object.
    /// @note The target will only be called if the timeout has not triggered.
    static Ptr Create(const std::string                 &name,
		      const aliLuaCore::CallTarget::Ptr &target,
		      const aliLuaCore::Exec::Ptr       &exec,
		      const aliLuaCore::MakeFn          &args,
		      const aliSystem::Time::TP         &utcTimeout,
		      const DepVec                      &dependencies);
    
    /// @brief GetName returns the name of the action
    /// @return the action's name
    const std::string                  &GetName  () const;
    
    /// @brief GetTarget returns the name of the action's target
    /// @return the action's target
    const aliLuaCore::CallTarget::Ptr  &GetTarget() const;
    
    /// @brief GetExec returns the exec to use to run the target
    /// @return the action's Exec
    const aliLuaCore::Exec::Ptr        &GetExec  () const;
    
    /// @brief GetResult fetches the result of the action
    /// @return A future that holds (or will hold) the result of the
    ///         call target's return.
    const aliLuaCore::Future::Ptr      &GetResult() const;
    
    /// @brief destructor
    ~Action();
    
    /// @brief ForEachDependency enable walking the list of
    ///        dependencies that define the Action's trigger criteria.
    /// @param fn is a function to recieve the dependencies.  When this
    ///        function is called, returning true will contine the
    ///        dependency iteration while returning false will halt it.
    void ForEachDependency(const DepFn &fn);
    
    /// @brief IsReady returns a flag indicating whether the dependency
    ///        criteria is met.
    /// @return ready flag
    bool IsReady() const;
    
    /// @brief HasRun returns a flag indicating whether the action
    ///        as executed.
    /// @return execution state flag
    bool HasRun() const;
    
    /// @brief SetTimeout can be called to flag the acton as time'd out
    ///
    /// By creeating the Action with an absurd timeout, one can manage
    /// the timeout behavior explicitly and independently of the Action
    /// class's logic.
    /// @note This call does nothing if the action has already run
    ///       or has been flagged with an error.
    void SetTimeout();
    
    /// @brief SetError can be called to trigger an error for the Action.
    /// @param err is the string to use as as the error message.
    /// @note This call does nothing if the action has already run
    ///       or has been flagged with an error.
    void SetError(const std::string &err);
    
    /// @brief The trigger function is used to trigger the evaluation
    ///        of the action's dependency criteria.  It is not necessary
    ///        to call this function.
    void Trigger();
    
    /// @brief Serialization function for Action objects.
    /// @param out is stream to which the object should be written
    /// @param o is the Action to serialize
    /// @return the stream passed as paramter out
    friend std::ostream &operator<<(std::ostream &out, const Action &o);
    
  private:
    /// @brief constructor
    Action();
    
    /// @brief RegisterDependencies builds the dependency list while
    ///        also registering to the dependncy object's OnSet listeners
    ///        container.
    static void RegisterDependencies(const Ptr &act);
    
    /// @brief An internal trigger used to evalute the state of the
    ///        Action's dependencies.
    /// @param wAct is a weak pointer to the action to evaluate
    /// @param dep is a pointer to the future that triggered the check.
    static void Trigger(const WPtr &wAct, aliLuaCore::Future *dep);
    
    std::mutex                  lock;           ///< mutex for various class members
    std::string                 name;           ///< name of the action
    aliLuaCore::CallTarget::Ptr target;         ///< target call
    aliLuaCore::Exec::Ptr       exec;           ///< target call's exec
    aliLuaCore::MakeFn          args;           ///< target call's args
    aliSystem::Time::TP         utcTimeout;     ///< utcTimeout
    aliLuaCore::Future::Ptr     result;         ///< Action's result
    DepVec                      dependencies;   ///< Action's dependencies
    bool                        hasRun;         ///< Action's execution status flag
  };
    
}

#endif
