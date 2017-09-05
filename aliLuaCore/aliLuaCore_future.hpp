#ifndef INCLUDED_ALI_LUA_CORE_FUTURE
#define INCLUDED_ALI_LUA_CORE_FUTURE

#include <aliLuaCore_staticObject.hpp>
#include <aliSystem.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace aliLuaCore {

  /// @brief The Future class is intended to enable capturing
  ///        results asynchronously.
  ///
  /// This class works in conjunction with the asynchronous
  /// execution of Lua code through the Exec class and its
  /// derivatives.
  ///
  /// Results are always prefixed with a true or false value
  /// indicating whether an error was set or whether the
  /// results were set.  Once a value or error is set, the
  /// object's value cannot be reset and any attempt to do
  /// so will trigger an exception.
  ///
  /// Although Lua supports returning an arbitrary value
  /// when triggering an error, this class only supports
  /// string value errors.  One reason for this is portability.
  /// An object is not as easyly translated if checked from
  /// C++.  Also, despite the ability for an application to
  /// handle one of many errors triggered by a function call
  /// in most cases, an API can be desiged to avoid complex
  /// exception handling logic.  For example, if one wants
  /// to create an API that might trigger many different types
  /// of exceptions, rather than throwing exceptions, that the
  /// caller would explicitly handle with some exception handling
  /// logic at the point of the call, the API can expose the
  /// ability for the caller to pass an exception handler for
  /// the types of exceptions that might occur.  In this way,
  /// the caller gets fine grained control of exceptions without
  /// a more complex exception object.
  struct Future {
    using Ptr  = std::shared_ptr<Future>; ///< shared pointer
    using WPtr = std::weak_ptr<Future>;   ///< weak pointer
    using OBJ  = StaticObject<Future>;    ///< Static object
    using LOBJ = StaticObject<aliSystem::Listener<Future*> >; ///< future listener

    /// @brief Initialize hold module
    /// @param cr is a component registry to which any initialzation
    ///        and finalization logic should be registered.
    /// @note This function should only be called from aliLuaCore::RegisterInitFini.
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
    
    /// @brief Create a future
    /// @return a shared pointer to a future
    static Ptr Create();

    /// @brief Create a future with the value pre-set.
    /// @param value the value to set
    /// @return a shared poniter to a future
    static Ptr Create(const MakeFn &value);
    
    /// @brief deconstructor
    ~Future();
    
    /// @brief SetValue is used to associate a value with the
    ///        future.
    /// @param value the value to set
    /// @note The value set by this call is not exactly what will
    ///       be returned by the GetValue in that a true value
    ///       will be prefixed to the value.
    /// @note Listeners will be notified if the value is not already
    ///       set.
    /// @note This function will throw an exception if the value has
    ///       already been set, or a timeout has been flagged, or an
    ///       error has been set.
    void SetValue(const MakeFn &value);
    
    /// @brief SetTimeout flags the Future instance with an error
    ///        and the string 'timeout'.
    /// @note Once this is called, listeners will be notifed unless
    ///       Future::SetValue or Future::SetError have already been
    ///       called for the instance.
    /// @note The value returned once this call is made will be
    ///       true, 'timeout'
    /// @note Once this is called, Future::GetError will return the
    ///       string timeout.
    void SetTimeout();
    
    /// @brief SetErro is used to associate an error with the Future.
    /// @param err is the error string to flag.
    /// @note This call will trigger the nofifiation of listeners
    ///       unless a previous call to SetValue or Timeout has
    ///       already been made.
    /// @note If SetValue or Timeout have previuosly been called, this
    ///       function will throw an exception.
    /// @note Once this call has been made, GetValue will return
    ///       false, err.
    void SetError(const std::string &err);
    
    /// @brief IsSet will return true if the instances Value has been
    ///        set or a timeout or error has been flagged.
    /// @return false if no error or value has been set true otherwise.
    bool IsSet() const;
    
    /// @brief IsError will return true if an error or timeout has
    ///        be set.
    /// @return error status
    bool IsError() const;
    
    /// @brief GetError will return an empty string unless an error
    ///        or timeout has been set, in which case the associated
    ///        error string will be returned.
    /// @return error string
    const std::string &GetError() const;
    
    /// @brief GetValue will return a MakeFn with the value that
    ///        has been associated with the Future instance.
    /// @return The value prefixed with true/false, which will
    ///         be true if SetValue was called, false if SetError or
    ///         SetTimeout was called.
    /// @note If no value has been associated with the Future, this
    ///       function will return an unset MakeFn.
    const MakeFn &GetValue() const;
    
    /// @brief OnSet will return a reference to the object's
    ///        listener.
    /// @return a constant reference to a pointer to a Future Listener.
    /// @note The returned value should always been a valid pointer.
    ///       If one wishes to ensure that they always verify the
    ///       pointer is valid before registring a listener, then
    ///       it is recommended that a caller use the static
    ///       Listener::Register functions, which provide that utility.
    const aliSystem::Listeners<Future*>::Ptr &OnSet();
    
  private:
    /// @brief constructor
    Future();
    
    /// @brief SetValueInternal is an internal function used to
    ///        set the value or error for a Future instance.
    ///
    /// This function is used to ensure that the verification that
    /// listeners are only triggered once and that there is no
    /// race between calls to SetError, SetValue, SetTimeout.
    ///
    /// @param value the value to set
    /// @param isError a flag indicating whether the value represents
    ///        an error value.
    /// @param error an empty string or an error message.
    void SetValueInternal(const MakeFn      &value,
			  bool              isError,
			  const std::string &error);
    
    MakeFn                             value;   ///< future's value
    bool                               isError; ///< future's error flag
    std::string                        error;   ///< future's error message
    aliSystem::Listeners<Future*>::Ptr onSet;   ///< future's listeners
  };

}

#endif
