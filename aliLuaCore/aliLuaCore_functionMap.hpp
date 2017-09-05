#ifndef INCLUDED_ALI_LUA_CORE_FUNCTION_MAP
#define INCLUDED_ALI_LUA_CORE_FUNCTION_MAP

#include <aliLuaCore_types.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <map>

namespace aliLuaCore {

  /// @brief A FunctionMap is a container that associates a funcction's
  ///        name to its implementation.
  struct FunctionMap {
    using Ptr    = std::shared_ptr<FunctionMap>; ///< shared pointer
    using FMap   = std::map<std::string, LuaFn>; ///< mapping
    using FFn    = std::function<bool(const std::string &name, const LuaFn &luaFn)>;  ///< an iterator function
    using WrapFn = std::function<int(const LuaFn &orig, lua_State *L)>; ///< a function wrapper

    /// @brief construct a function map
    /// @param name of the map
    static Ptr Create(const std::string &name);
    
    /// @brief destructor
    ~FunctionMap();
    
    /// @brief retrieve the map's name
    /// @brief return the name of the map
    const std::string &Name() const;
    
    /// @brief retrive the map's status
    /// @return return a flag indicating whether the map is frozen
    /// @note once frozen, the FunctionMap can no longer be manipulated
    bool IsFrozen() const;
    
    /// @brief determine if a function is defined in the map
    /// @param fnName is the name of the function to check
    /// @return a flag, where true indicates the function is defined
    ///         within the map.
    bool IsDefined(const std::string &fnName);
    
    /// @brief Freeze the FunctionMap
    /// @note Once frozen, attempts to change the FunctionMap will trigger exceptions
    void Freeze();
    
    /// @brief function accessor
    /// @param fn is a function to enable processing each function
    /// @note If called function returns false, the iteration will cease.
    ///       Returning true will allow iterating through all functions.
    void ForEach(const FFn &fn);
    
    /// @brief Add a function to the map
    /// @param fnName is the name to represent the function
    /// @param luaFn is the function to associate with the name
    /// @note The LuaFn is a functor, not a function address or reference.
    ///       One may bind arguments or use a object variables to carry
    ///       data with the function.
    /// @note If the function already exists, an exception will be thrown.
    void Add(const std::string &fnName, const LuaFn &luaFn);
    
    /// @brief Define an alias to an existing function.
    /// @param alias is the new name to also associate with the original function
    /// @param orig is the name of the function that one wishes to alias.
    /// @note A common use for alias is to alias a metatable's GC function with
    ///       an explicit Release function to allow script logic to immediately
    ///       free resources associated with the object rather than having to wait
    ///       for the garbage collector to collect the object and free the
    ///       resources.
    void Alias(const std::string &alias, const std::string &orig);
    
    /// @brief Define a wrapper function.
    ///
    /// When a function is wrapped, the wrapper will be called instead of the
    /// original funciton.  The wrapper function will recieve the original function
    /// as an argument.
    ///
    /// If one "wraps" a function that is not defined, the call will succeed.
    /// In this case when the wrapper function is called, the argument representing
    /// the original function will be an uninitialized functor.  Calling it will
    /// trigger an exception.  If one wishes to avoid this, they should verify
    /// the function exists prior to wrapping it and if it does not, define a
    /// no-op type function, then alias that function.
    ///
    /// @param fnName the name of the wrapper function.
    /// @param wrapFn is the functor that should be called in place of the original.
    ///        The original function (if defined) will be passed to the wrapper
    ///        function.
    void Wrap(const std::string &fnName, const WrapFn &wrapFn);
    
  private:
    /// @brief constructor
    FunctionMap();
    
    std::mutex  lock;       ///< mutex protecting edits to the fMap
    std::string name;       ///< name of the FunctionMap
    FMap        fMap;       ///< map for holding the functions
    bool        isFrozen;   ///< flag indicating the FunctionMap's status
  };
  
}


#endif
