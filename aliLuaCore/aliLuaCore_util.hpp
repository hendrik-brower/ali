#ifndef INCLUDED_ALI_LUA_CORE_UTIL
#define INCLUDED_ALI_LUA_CORE_UTIL

#include <aliLuaCore_types.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aliLuaCore {
  struct Exec;
  struct Future;

  /// @brief This class defines a set of functions that simply common calls to Lua interperters.
  struct Util {
    using SVec      = std::vector<std::string>;  ///< String vector
    using ExecPtr   = std::shared_ptr<Exec>;     ///< Shared pointer to a Lua Exec
    using FuturePtr = std::shared_ptr<Future>;   ///< Shared pointer to a future

    // ****************************************************************************************
    // miscelaneous
    /// @brief RCStr returns a string representation of a Lua return code.
    /// @param rc is a Lua return code
    /// @return string describing the return code.  EG: LUA_OK, LUA_ERRERR, ...
    static std::string RCStr(int rc);
    
    // ****************************************************************************************
    // exec based calls
    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param fn is the function to run.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void Run(const ExecPtr &ePtr,
		    const LuaFn   &fn);

    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn(const ExecPtr     &ePtr,
		      const std::string &fnName);
    
    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a list of strings to pass as the arguments to the function.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.

    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn(const ExecPtr     &ePtr,
		      const std::string &fnName,
		      const SVec        &args);
    
    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a make function that wraps up 0 or more arguemnts to pass to the funtion.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn(const ExecPtr     &ePtr,
		      const std::string &fnName,
		      const MakeFn      &args);
    
    /// @brief Execute the specified function for the specified Lua object through the
    ///        specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param obj is a make function that should create the object for which the specified
    ///        function should be called.
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a make function that wraps up 0 or more arguemnts to pass to the funtion.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunMT(const ExecPtr     &ePtr,
		      const MakeFn      &obj,
		      const std::string &fnName,
		      const MakeFn      &args);
    
    /// @brief LoadFile will load the script from the specified file into the specified Lua exec.
    /// @param ePtr is the Lua exec
    /// @param path is the absolute or relative path name for the script to load
    /// @note Any return value returned from the loaded script is discarded.
    /// @note If the file cannot be loaded, an exception will be thrown when the Lua exec runs
    ///       the load.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the file may not be loaded when this function returns.
    static void LoadFile(const ExecPtr     &ePtr,
			 const std::string &path);
    
    /// @brief LoadString will load the script from the string into the specified Lua exec.
    /// @param ePtr is the Lua exec
    /// @param str is a script to load
    /// @note Any return value returned from the loaded script is discarded.
    /// @note If the script cannot be loaded, an exception will be thrown when the Lua exec runs
    ///       the load.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the script may not be loaded when this function returns.
    static void LoadString(const ExecPtr     &ePtr,
			   const std::string &str);
    
    /// @brief LoadFnMap will load a table of functions to a table defined by the path.
    ///        The path will be constructed if it does not exist.  The path may be composed
    ///        of multiple tables, though all elements are treated as string keys.
    ///        Thus, loosely speaking:
    ///            LoadFnMap(e, "abc.def", { "a" -> fnA, "b" -> fnB })
    ///        will create the table abc with a sub table def with two keys a and b.
    /// @param ePtr is the Lua exec
    /// @param path is the function map's base table
    /// @param fnMap is a map containing a set of function names and LuaFn handlers.
    /// @note If one of the table path elements exists and is anything but a table, it will be
    ///       overwritten with a newly created table.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the functions may not be loaded when this function returns.
    static void LoadFnMap(const ExecPtr          &ePtr,
			  const std::string      &path,
			  const FunctionMap::Ptr &fnMap);

    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param fn is the function to run.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void Run(const ExecPtr   &ePtr,
		    const FuturePtr &future,
		    const LuaFn     &fn);
    
    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn (const ExecPtr     &ePtr,
		       const FuturePtr   &future,
		       const std::string &fnName);

    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a list of strings to pass as the arguments to the function.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn(const ExecPtr     &ePtr,
		      const FuturePtr   &future,
		      const std::string &fnName,
		      const SVec        &args);
    
    /// @brief Execute the specified function through the specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a make function that wraps up 0 or more arguemnts to pass to the funtion.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunFn(const ExecPtr     &ePtr,
		      const FuturePtr   &future,
		      const std::string &fnName,
		      const MakeFn      &args);

    /// @brief Execute the specified function for the specified Lua object through the
    ///        specified Lua interpreter.
    /// @param ePtr is the Lua exec in which the function should run
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param obj is a make function that should create the object for which the specified
    ///        function should be called.
    /// @param fnName is the global name of the function to run.  GetElement is used to resovle
    ///        the function, so it may be in a subtable (eg xyz.fnList[1], or abc.fnGroups.fnName).
    /// @param args is a make function that wraps up 0 or more arguemnts to pass to the funtion.
    /// @note Any return value from the passed function is discarded.
    /// @note The function may be a bare c/c++ function, a c++ closure, a c++ functor.
    /// @note It is ok to throw an exception in the passed function when it executes.
    ///       Any exception that is thrown will be logged and ignored.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the function call will not be made (or complete) when this function returns.
    static void RunMT(const ExecPtr     &ePtr,
		      const FuturePtr   &future,
		      const MakeFn      &obj,
		      const std::string &fnName,
		      const MakeFn      &args);
    
    /// @brief LoadFile will load the script from the specified file into the specified Lua exec.
    /// @param ePtr is the Lua exec
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param path is the absolute or relative path name for the script to load
    /// @note Any return value returned from the loaded script is discarded.
    /// @note If the file cannot be loaded, an exception will be thrown when the Lua exec runs
    ///       the load.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the file may not be loaded when this function returns.
    static void LoadFile(const ExecPtr     &ePtr,
			 const FuturePtr   &future,
			 const std::string &path);
    
    /// @brief LoadString will load the script from the string into the specified Lua exec.
    /// @param ePtr is the Lua exec
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param str is a script to load
    /// @note Any return value returned from the loaded script is discarded.
    /// @note If the script cannot be loaded, an exception will be thrown when the Lua exec runs
    ///       the load.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the script may not be loaded when this function returns.
    static void LoadString(const ExecPtr     &ePtr,
			   const FuturePtr   &future,
			   const std::string &str);
    
    /// @brief LoadFnMap will load a table of functions to a table defined by the path.
    ///        The path will be constructed if it does not exist.  The path may be composed
    ///        of multiple tables, though all elements are treated as string keys.
    ///        Thus, loosely speaking:
    ///            LoadFnMap(e, "abc.def", { "a" -> fnA, "b" -> fnB })
    ///        will create the table abc with a sub table def with two keys a and b.
    /// @param ePtr is the Lua exec
    /// @param future is a future to which any result(s) are set.  If an exception is thrown, the
    ///        future will be marked with an error.
    /// @param path is the function map's base table
    /// @param fnMap is a map containing a set of function names and LuaFn handlers.
    /// @note If one of the table path elements exists and is anything but a table, it will be
    ///       overwritten with a newly created table.
    /// @note It is possible the Lua Exec will queue the execution of the function, in which case
    ///       the functions may not be loaded when this function returns.
    static void LoadFnMap (const ExecPtr          &ePtr,
			   const FuturePtr        &future,
			   const std::string      &path,
			   const FunctionMap::Ptr &fnMap);

    // ****************************************************************************************
    // bare Lua calls

    /// @brief GetElement will retrieve an global value that is potentially
    ///        contained under 0 or more subtables.
    ///
    ///   Retrieve an element from the global state that can be accessed via
    ///   the passed string.  The found element will be left on the top of the
    ///   stack.  While recursing to the element, if a table element is not
    ///   found at an intermediate node, the function will throw an exception.
    ///
    ///   If key includes . or [] characters, the function will found by extracting
    ///   the function from the noted table. Bracketed numbers will be looked up as
    ///   integer keys. Bracketed strings (which would be variable lookups in Lua)
    ///   are expanded.  Any 'dot' based component of the function's path will be
    ///   treated as a string (even if its numeric).
    ///
    ///   Example keys:
    ///         myTbl[200].subKey.MyFn -- returns a function
    ///         myTbl[200]             -- returns a table
    ///
    /// @param L is the Lua State from which the value should be sought.
    /// @param key is a path to the sought value
    /// @return The type of the element is returned from this call.  EG: LUA_TNIL,
    ///         LUA_TSTRING, ...
    /// @note This function will throw an exception if the key string is not entirely made up
    ///       of characters from: . [ ] a-Z 0-9
    static int GetElement(lua_State         *L,
			  const std::string &key);

    /// @brief Load functors will define a set of functions with their associated names
    ///        as the keys in the specified table.
    ///
    ///        The passed functions are wrapped and held in a shared pointer that is
    ///        placed in the given registry.  Wrapped functions are pushed into the Lua
    ///        interpreter with a common function handler and an value that points to
    ///        the wrapper.  In testing, it appeared as though upvalues for functions
    ///        that have garbage collectors (eg user data up values), do not get their
    ///        __gc routines called when the function is released (or even when the
    ///        interpreter is closed).  Thus, the function wrappers are pushed as
    ///        light user data up values.  The registry ensures that the wrapper's life
    ///        cycle can be effectively managed.  In general, a Lua Exec's Registry
    ///        should be used, though if one is using this against a private interperter
    ///        defined outside of a Exec object, then they should also define and maintain
    ///        a registry who's life exceeds the interpreter's life.
    ///
    ///        The base name is used to track statistics for the function calls with a
    ///        friendly name composed as baseName.fnName.
    ///
    /// @param rPtr is a registry to hold function wrappers
    /// @param L the Lua interpreter to which the functions should be added
    /// @param tblIdx a table to which the functions should be added.
    /// @param baseName a prefix used to more uniquely define the function name
    ///        (other than the function named defined by the fnMap).
    /// @param fnMap is the list of function names and their assoicated handlers
    /// @note This function will throw an exception if tblIdx does not refer to
    ///       a table.  It will also throw exceptions if the rPtr is null.
    static void LoadFnMap(const aliSystem::Registry::Ptr &rPtr,
			  lua_State                      *L,
			  int                             tblIdx,
			  const std::string              &baseName,
			  const FunctionMap::Ptr         &fnMap);


    /// @brief GetRegistryKey returns a unique number that is incremented with each call.
    ///
    ///        This value can be used to define items within the LUA_REGISTRYINDEX as long
    ///        as the function is not called more than N times where N is Lua's table index
    ///        limit and as long as items are not put in table with integer keys defined
    ///        elsewhere that may overlap with the numbers returned by this function.
    /// @return a registry key (index)
    /// @note The number of items kept in the registry should be reasonably small to ensure
    ///       Lua's registry table does not grow too large.  Lua's implementation for
    ///       integer keys is not specifically defined, so if a spare table is used,
    ///       storing non-complete sets of values will not cause excessive unused vector
    ///       elements.  However, if a non-spare table is used, it will.  Thus if one
    ///       creates 1000 interperters and defines 1000 items in each, and if the Lua tables
    ///       are non-spare, then at least one interpreter will contain 10000 keys, of which
    ///       999000 will be to nil.
    static int GetRegistryKey();
    
    /// @brief Set the value of a registry key to the value on the top of the stack.
    ///        This top value is popped from the stack.
    /// @param L the interpreter for which the registry should be set
    /// @param key is the index to use to push the value
    /// @note In the future, the place where this function holds values and where
    ///       RegistryGet retrives values may change, so if one is using this to
    ///       store values, they should also always use the RegistryGet to retrieve those
    ///       values.
    static void RegistrySet(lua_State *L, int key);
    
    /// @brief Get the value of a registry item. The item will be placed on the top of the
    ///        stack.
    /// @param L the interpreter for which the registry should be set
    /// @param key is the index to use to push the value
    /// @note In the future, the place where the RegistrySet function holds values and where
    ///       this function retrives values may change, so if one is using RegistrySet to
    ///       store values, they should also always use this function to retrieve those
    ///       values.
    static void RegistryGet(lua_State *L, int key);
    
  };
  
}

#endif
