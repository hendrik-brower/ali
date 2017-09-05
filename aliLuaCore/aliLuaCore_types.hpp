#ifndef INCLUDED_ALI_LUA_CORE_TYPES
#define INCLUDED_ALI_LUA_CORE_TYPES

#include <functional>
#include <memory>
#include <vector>

struct lua_State;
namespace aliLuaCore {

  /// @brief A LuaFn is a generic functor for executing Lua logic.
  ///
  ///        Although it is a functor and cannot be passed to a Lua
  ///        function directly (like lua_pushcfunction), C++ wrappers
  ///        can use upvalues to associate the funtor with the
  ///        function, which allows a more flexible API in the context
  ///        of a C++ codebase.
  ///
  ///        The LuaFn should return an integer value greater than zero
  ///        and that value should reflect the number of items pushed
  ///        onto the Lua stack.
  using LuaFn     = std::function<int(lua_State*)>;

  /// @brief A WrapperFn is a LuaFn function that also has a LuaFn
  ///        as an argument.
  ///
  ///        The primary use for this function is to allow wrapping
  ///        execution logic before and/or after the wrapped LuaFn.
  ///        The returned function can only be used by API's that
  ///        specifically support it rather than the more common
  ///        LuaFn.
  using WrapperFn = std::function<int(const LuaFn &, lua_State *)>;

  /// @brief A MakeFn is a generic function for executing logic from
  ///        a Lua interpreter.  The only difference from a LuaFn is
  ///        that its intended to imply a function that pushed a value
  ///        onto a Lua stack.
  ///
  ///        In most cases, a MakeFn will push one value on the stack
  ///        and should return the value 1.  However, it is also
  ///        resaonble to push more than one value and return a count
  ///        of objects pushed.
  ///
  ///        Unlike Lua where builing a table with something of the
  ///        form: local tbl = { Fn1(), Fn2(), Fn3() }, will cause
  ///        both Fn1 and Fn2 to truncate their result to a single
  ///        value, but will include as many values as Fn3 returns,
  ///        the common use of MakeFn within C++ does not use this
  ///        approach, but instead pushes all arguments fromo each
  ///        function.  If a function does not push any arguments,
  ///        then its predecessor's and successor's values will be
  ///        directly abutting rather than having a nil value
  ///        sqweezed between them.
  using MakeFn    = std::function<int(lua_State*)>;

  /// @brief A vector of MakeFn's.  See MakeFn for more information.
  using MakeFnVec = std::vector<MakeFn>;

  /// @brief A pair of MakeFn's.
  ///
  ///        This struture can logically repesent
  ///        a key value pair for a map, though in this case, both
  ///        MakeFn's should return single values and the first should
  ///        not return nil (an invalid table key).
  ///
  ///        When used to represent a table's key/value pair, the first
  ///        value is treated as the key and the second as the value.
  using KVPair  = std::pair<MakeFn, MakeFn>;

  /// @brief A vector of MakeFn's.  This struture can represent an
  ///        arbitrarily complex Lua table since the value MakeFn's could
  ///        represent an entire subtree.
  using KVVec   = std::vector<KVPair>;
  
  /// @brief KVFn is merely a function that takes two MakeFn values.
  ///
  ///        As an API argument, it is usually intented to communicate
  ///        that the function will generate a key/value pair suitable
  ///        for construting a Lua table.
  ///        In this context, both functinos should return exacctly one
  ///        value and the first should not return nil.
  using KVFn = std::function<void(const MakeFn &, const MakeFn &)>;
  
}

#endif
