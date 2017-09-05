#include <aliLuaCore_stackGuard.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>

namespace aliLuaCore {

  StackGuard::StackGuard(lua_State *L_)
    : L(L_),
      top(lua_gettop(L_)) {
  }
  StackGuard::StackGuard(lua_State *L_, int checkStackSize)
    : StackGuard(L_) {
    lua_checkstack(L, checkStackSize);
  }
	  
  StackGuard::~StackGuard() {
    Clear();
  }
  int StackGuard::Index(int offset) const { return top+offset; }
  void StackGuard::Verify(int diff, const std::string &msg) {
    int cur = lua_gettop(L);
    THROW_IF(cur-top!=diff, "Stack guard verify error, expecting " << diff
	     << " got " << cur-top << ", msg=" << msg);
  }
  int StackGuard::Diff() {
    return lua_gettop(L)-top;
  }
  void StackGuard::Clear() {
    int diff = Diff();
    if (diff>0) {
      lua_pop(L, diff);
    }
  }

}
