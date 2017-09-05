#ifndef INCLUDED_ALI_LUA_CORE_VALUES
#define INCLUDED_ALI_LUA_CORE_VALUES

#include <aliLuaCore_types.hpp>


struct lua_State;
namespace aliLuaCore {

  /// @brief The Values structure defines utility functions related to
  ///        creating, extracting, or manipulating values in a Lua state.
  ///
  /// The api is essentially divided into two halves.  The functions that
  /// immediately manipulate the Lua state and those that return functions
  /// that when called with a Lua state argument, modify that state.
  /// These returned functions retain the values when they are created.
  /// These retained values are released when the returned function is
  /// destroyed.  Many functions retain basic values; however, in the case
  /// of functions that are composed of potentially more complex data
  /// types, one may need to consider the life cycle of the returned
  /// MakeFn.  Use of std::shared_ptr's within these objects can prevent
  /// memory from being released until later.  In cases where objects are
  /// shared by a Lua interpreter, it is very easy to create circular
  /// references that will prevent the release of memory.  Use of weak
  /// pointers in MakeFn's is often desiarable.  Even so, there are times
  /// where holding a shared pointer is the correct approach.
  ///
  struct Values {

    /// @brief Extract a string from the Lua state.
    /// @param L Lua state
    /// @param index is the index to extract
    /// @note Non-strings that can be converted to strings are converted.
    ///       This is done without altering the value at the passed index.
    /// @note An empty string is returned for values that cannot be converted
    ///       to a string.  This includes the following types:
    ///          LUA_TNIL, LUA_TTABLE, LUA_TFUNCITON, LUA_TTHREAD
    ///       User data that defines a tostring function will be converted
    ///       according to the object's tostring function.
    static std::string GetString(lua_State *L, int index);

    
    // ****************************************************************************************
    // Constructors
    
    /// @brief Make nothing does just that... nothing.
    /// @param L Lua state
    /// @return 0, which is the number of items pushed onto the interpreters stack.
    static int MakeNothing    (lua_State *L);

    /// @brief Push a nil value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeNil        (lua_State *L);

    /// @brief Push a true value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeTrue       (lua_State *L);

    /// @brief Push a false value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeFalse      (lua_State *L);

    /// @brief Push an integer value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @param val is the integer to push into the Lua state.
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeInteger    (lua_State *L, int    val);

    /// @brief Push a double value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @param val is the double to push into the Lua state.
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeDouble     (lua_State *L, double val);

    /// @brief Push a string value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @param val is the string to push into the Lua state.
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeString     (lua_State *L, const std::string &val);

    /// @brief Push an empty string value onto the given Lua interpreter's stack.
    /// @param L Lua state
    /// @return 1, which is the number of items pushed onto the interpreters stack.
    static int MakeEmptyString(lua_State *L);

    // ****************************************************************************************
    // construction API
    /// @brief Return a MakeFn function that when called will push nothing into the given Lua
    ///        interpreter.
    /// @return A MakeFn that will do nothing to the pased Lua interpreter when it is called.
    static MakeFn GetMakeNothingFn();

    /// @brief Return a MakeFn function that when called will push nil into the given Lua
    ///        interpreter.
    /// @return A MakeFn that will push nil into the pased Lua interpreter when it is called.
    static MakeFn GetMakeNilFn    ();

    /// @brief Return a MakeFn function that when called will push a boolean into the given Lua
    ///        interpreter.
    /// @param  val is the boolean value to wrap in the returned MakeFn.
    /// @return A MakeFn that will push the passed boolean value into the pased Lua interpreter
    ///         when it is called.
    static MakeFn GetMakeBoolFn   (bool val);

    /// @brief Return a MakeFn function that when called will push true into the given Lua
    ///        interpreter.
    /// @return A MakeFn that will push true into the pased Lua interpreter when it is called.
    static MakeFn GetMakeTrueFn   ();

    /// @brief Return a MakeFn function that when called will push false into the given Lua
    ///        interpreter.
    /// @return A MakeFn that will push false into the pased Lua interpreter when it is called.
    static MakeFn GetMakeFalseFn  ();

    /// @brief Return a MakeFn function that when called will push an integer into the given Lua
    ///        interpreter.
    /// @param  val is the integer value to wrap in the returned MakeFn.
    /// @return A MakeFn that will push the passed integer into the pased Lua interpreter when
    ///         it is called.
    static MakeFn GetMakeIntegerFn(int    val);

    /// @brief Return a MakeFn function that when called will push a double into the given Lua
    ///        interpreter.
    /// @param  val is the double value to wrap in the returned MakeFn.
    /// @return A MakeFn that will push the passed double into the pased Lua interpreter when
    ///         it is called.
    static MakeFn GetMakeDoubleFn(double val);

    /// @brief Return a MakeFn function that when called will push a string into the given Lua
    ///        interpreter.
    /// @param  val is the string value to wrap in the returned MakeFn.
    /// @return A MakeFn that will push the passed string into the pased Lua interpreter when
    ///         it is called.
    static MakeFn GetMakeStringFn (const std::string &val);

    /// @brief Return a MakeFn function that when called will push an empty string into the
    ///        given Lua interpreter.
    /// @return A MakeFn that will push an empty string into the pased Lua interpreter when
    ///         it is called.
    static MakeFn GetMakeEmptyStringFn();

    /// @brief Return a MakeFn function that when called will push an array into the given Lua
    ///        interpreter.
    /// @param  mVec is a vector of zero or more MakeFn values to push into the interpreter
    ///         when the returned make funcion is called.
    /// @return a MakeFn that will push N items into the given Lua interpreter when it is called.
    /// @note The number of items pushed into the Lua state (N), is not necessarily the same as
    ///       the number of items in the given MakeFnVec.  Each item in the MakeFnVec, might
    ///       push 0 or more items.
    /// @note Though this function is in the aliLuaCore::Values namespace, which aims to provide
    ///       utility for transfering values in and out of an interpreter, there is no real
    ///       limitation of what is contained in the make functions contained by the mVec
    ///       parameter.  It is possible to use this function to wrap an arbitrary sequence
    ///       of calls to an interpreter.  The returned value will merely be the sum of
    ///       each of the returned values by the individual MakeFn functions.  Regardless
    ///       of this logical capability, one might be careful in their application design
    ///       to make sure developers understand the intent of less intuitive uses.
    static MakeFn GetMakeArrayFn(const MakeFnVec &mVec);

    // ****************************************************************************************
    // extraction API
    /// @brief GetMakeFnForAll will return a make function that will extract a make function
    ///        that will re-create the current stack for the passed Lua state.
    /// @param L the Lua state from which values should be extracted.
    /// @return A MakeFn that will push the wrapped up values from when this function was called.
    /// @note When a MakeFn is called that pushes values into a Lua interperter, these values
    ///       are always appended to the existing stack.  The function returned by a call to
    ///       GetMakeFnForAll, will not clear the stack, and then push the values.
    /// @note At present, this function will recursively extract tables, but will not capture
    ///       associated meta tables.  Because of this, use of native Lua objects that flow
    ///       in and out of in interpreter through these functions will loose the object
    ///       definition and simply be handled as pure data tables.  To define "passable"
    ///       Lua objects, please refer to the ExternalMT and ExternalObject APIs.
    /// @note This function will throw an exception if one attemts to use it to extract items that
    ///       cannot be extracted (eg non-duplicable ali::Object, a Lua function).
    static MakeFn GetMakeFnForAll(lua_State *L); // all stack values

    /// @brief GetMakeFnForIndex will return a make function that will extract a make function
    ///        that will re-create the identified stack index value for the passed Lua state.
    /// @param L the Lua state from which values should be extracted.
    /// @param index the stack position to be extracted.
    /// @return A MakeFn that will push the wrapped up values from when this function was called.
    /// @note When a MakeFn is called that pushes values into a Lua interperter, these values
    ///       are always appended to the existing stack.  The function returned by a call to
    ///       GetMakeFnForIndex, will not clear the stack, and then push the values.  Nor will
    ///       it replace the current value at the specified index with the previously extracted
    ///       value.
    /// @note At present, this function will recursively extract tables, but will not capture
    ///       associated meta tables.  Because of this, use of native Lua objects that flow
    ///       in and out of in interpreter through these functions will loose the object
    ///       definition and simply be handled as pure data tables.  To define "passable"
    ///       Lua objects, please refer to the ExternalMT and ExternalObject APIs.
    /// @note This function will throw an exception if one attemts to use it to extract items that
    ///       cannot be extracted (eg non-duplicable ali::Object, a Lua function).
    static MakeFn GetMakeFnForIndex(lua_State *L, int index); // specific stack value

    /// @brief GetMakeFnByCount will return a make function that will extract a specified number of
    ///        stack values starting at a given offset and wrap them into a MakeFn that is returned.
    /// @param L the Lua state from which values should be extracted.
    /// @param index the first stack position to be extracted.
    /// @param count a number ranging from zero to MAX_INT, which will specifiy how many stack
    ///        values to extract.
    /// @return A MakeFn that will push the wrapped up values from when this function was called.
    /// @note index may be a negative number, in which case it is computed as the stack value
    ///       down from the top of the stack.  A index  of -2 will and a count of 1 will extract
    ///       the second to last value.  A index of -2 and a count of 2 or more will simply return
    ///       a function that will wrap the last two stack values.
    /// @note When a MakeFn is called that pushes values into a Lua interperter, these values
    ///       are always appended to the existing stack.  The function returned by a call to
    ///       GetMakeFnForIndex, will not clear the stack, and then push the values.  Nor will
    ///       it replace the same range that was originally extracted.
    /// @note At present, this function will recursively extract tables, but will not capture
    ///       associated meta tables.  Because of this, use of native Lua objects that flow
    ///       in and out of in interpreter through these functions will loose the object
    ///       definition and simply be handled as pure data tables.  To define "passable"
    ///       Lua objects, please refer to the ExternalMT and ExternalObject APIs.
    /// @note This function will throw an exception if one attemts to use it to extract items that
    ///       cannot be extracted (eg non-duplicable ali::Object, a Lua function).
    static MakeFn GetMakeFnByCount(lua_State *L,
			    int        index,
			    size_t     count); // range of stack values

    /// @brief GetMakeFnRemaining will return a make function that will extract the stack values
    ///        from the specified index to the end of the stack.
    /// @param L the Lua state from which values should be extracted.
    /// @param index the first stack position to be extracted.
    /// @return A MakeFn that will push the wrapped up values from when this function was called.
    /// @note index may be a negative value, which will act to specify a stack position from
    ///       the end.  For example, -1 would be the last value on the stack.
    /// @note If index is zero, this will extract everything "passed the top of the stack", which
    ///       is nothing.
    /// @note When a MakeFn is called that pushes values into a Lua interperter, these values
    ///       are always appended to the existing stack.  The function returned by a call to
    ///       GetMakeFnForIndex, will not clear the stack, and then push the values.  Nor will
    ///       it replace the same range that was originally extracted.
    /// @note At present, this function will recursively extract tables, but will not capture
    ///       associated meta tables.  Because of this, use of native Lua objects that flow
    ///       in and out of in interpreter through these functions will loose the object
    ///       definition and simply be handled as pure data tables.  To define "passable"
    ///       Lua objects, please refer to the ExternalMT and ExternalObject APIs.
    /// @note This function will throw an exception if one attemts to use it to extract items that
    ///       cannot be extracted (eg non-duplicable ali::Object, a Lua function).
    static MakeFn GetMakeFnRemaining(lua_State *L, int index);

    /// @brief This function takes a make funtion that generates zero or more values and returns a MakeFn that
    ///        will generate a sequence with the original MakeFn's output.
    /// @param makeFn the make function whose values will be wrapped inside the returned MakeFn's table.
    /// @return a MakeFn that will genrate a table containing any values constructed by the passed MakeFn.
    /// @note If the passed make function does not produce a sequence (ie, it generates a nil value before other
    ///       non-nil values, the resulting table will be created, but will not actually be a sequence.
    static MakeFn ConstructAsSequence(const MakeFn &makeFn);

  };
  
}

#endif
