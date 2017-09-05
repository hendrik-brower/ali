#ifndef INCLUDED_ALI_LUA_CORE_STACK_GUARD
#define INCLUDED_ALI_LUA_CORE_STACK_GUARD

#include <string>

struct lua_State;
namespace aliLuaCore {

  /// @brief StackGuard provides a tool for automatically
  ///        removing any elements pushed onto the Lua stack after this
  ///        object is created and before it is destroyed.
  ///
  /// In general, this makes it relatively easy to work with the stack
  /// but ensure it is returend in the original state.
  /// @note Nothing in this class prevents values existing on the stack
  ///       or anywhere else inside the Lua Interpreter from being
  ///       manipulated, so using this fucntion does not guarentee that
  ///       the stack will be unaltered.  It also does not guarentee that
  ///       the stack will be the same size as it was when the stack guard
  ///       was created, only that it will be equal or less than the
  ///       original size.
  struct StackGuard {

    /// @brief unsupported constructor
    StackGuard(const StackGuard &) = delete;

    /// @brief unsupported constructor
    StackGuard operator=(const StackGuard &) = delete;

    /// @brief constructor
    /// @param L Lua state to guard
    explicit
    StackGuard(lua_State *L);

    /// @brief construtor
    /// @param L Lua State to guard
    /// @param checkStackSize is the minimum remaining stack space
    ///        to ensure when creating this object.
    /// @note This construtor is essentially the same as:
    ///       StackGuard(L);
    ///       lua_checkstack(L,checkStackSize);
    ///       Since that is a common pattern, this class was extended
    ///       to support this.
    StackGuard(lua_State *L, int checkStackSize);

    /// @brief Index returns the index relative to the top of the stack
    ///        when this object was created.
    ///
    ///        Thus, one can do something
    ///        like:
    ///           StackGuard g(L);
    ///           lua_newtable(L);
    ///        and know that g.Index(1) refers to the index of the newly
    ///        created table.  The returned value will be an absolute
    ///        stack index.
    /// @param offset the value to offset from the original top
    /// @return the absolute index of the top+offset
    int Index(int offset) const;

    /// @brief Verify the difference between the current top and the
    ///        top at the time the guard created match the expected value.
    ///
    /// If the stack does not differ as expected, a runtime_error is thrown
    /// with given msg.
    /// @param diff expected stack difference
    /// @param msg message to throw if the stack difference is not as
    ///        expected
    void Verify(int diff, const std::string &msg);

    /// @brief Diff returns the difference from the original top and
    ///        the current top.
    /// @return the difference, which may be negative it the stack has
    ///         shrunk since the guard was created.
    int Diff();

    /// @brief remove any times above the top at the time the guard
    ///        was created.
    void Clear();

    /// @brief destructor
    ~StackGuard();

  private:
    
    lua_State *L;   ///< Lua state
    int        top; ///< original stack size
  };
    
}

#endif

