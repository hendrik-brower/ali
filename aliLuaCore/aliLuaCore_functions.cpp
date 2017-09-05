#include <aliLuaCore_functions.hpp>

namespace aliLuaCore {

  LuaFn Functions::Wrap(LuaFn orig, WrapperFn fn) {
    return [=](lua_State *L) -> int {
      return fn(orig, L);
    };
  }

}
