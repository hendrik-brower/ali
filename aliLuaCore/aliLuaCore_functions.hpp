#ifndef INCLUDED_ALI_LUA_CORE_FUNCTIONS
#define INCLUDED_ALI_LUA_CORE_FUNCTIONS

#include <aliLuaCore_types.hpp>
#include <aliLuaCore_functionMap.hpp>
#include <map>
#include <memory>
#include <string>

struct lua_State;
namespace aliLuaCore {

  /// @brief Functions defines a number of utility functions
  ///        for wrappnig Lua functions/functors.
  struct Functions {

    /// @brief Wrap provides a method for wrappinng a Lua function
    ///        with another function that will be passed to the
    ///        wrapper.  This allows attaching logic before and or
    ///        after the original call.
    /// @param orig is the original function to wrap
    /// @param fn is the wrapper function to use
    /// @return The wrapped Lua functor.
    static LuaFn Wrap(LuaFn orig, WrapperFn fn);

    /// @brief Wrap provides a method of wrapping a shared pointer
    ///        with a Lua function/functor.
    /// @tparam T is the type of the shared pointer to to wrap
    /// @param itemPtr is the shared pointer to wrap with the
    ///        Lua functor.
    /// @param fn is the functor to call from the returned
    ///        function.  The signautre of this functor is
    ///        the same as a Lua funciton except the first argument
    ///        is a shared pointer (itemPtr).
    /// @return Lua functor
    template <typename T>
    static LuaFn Wrap(std::shared_ptr<T>                     itemPtr,
		      std::function<int(std::shared_ptr<T>,
					lua_State *)>        fn) {
      return [=](lua_State *L) -> int {
	return fn(itemPtr, L);
      };
    }

    /// @brief Wrap provides the ability to wrap an existing Lua
    ///        function/functor with another function and a value
    ///        contained within a shared pointer.
    /// @tparam ITEM is the type for the encapsulated shared ponter.
    /// @param itemPtr is the pointer to encapsulate
    /// @param orig is the original Lua function
    /// @param fn is the function wrapper to call.  This function
    ///        has the same signature as a Lua function except
    ///        a shared poiter (itemPtr) and the original Lua function
    ///        (orig) will be passed as the first two arguments.
    /// @return a Lua functor that will trigger a call to the wrapper.
    template <typename ITEM>
    static LuaFn Wrap(std::shared_ptr<ITEM>                     itemPtr,
		      LuaFn                                     orig,
		      std::function<int(std::shared_ptr<ITEM>,
					const LuaFn &,
					lua_State*)>            fn) {
      return [=](lua_State *L) -> int {
	return fn(itemPtr, orig, L);
      };
    }

  };
  
}

#endif
