#include <aliLuaExt_execPoolItem.hpp>

namespace aliLuaExt {

  ExecPoolItem::ExecPoolItem(const aliLuaCore::Future::Ptr &future_,
			     const aliLuaCore::LuaFn       &luaFn_)
    : future(future_),
      luaFn(luaFn_) {
  }
  const aliLuaCore::Future::Ptr &ExecPoolItem::GetFuture() const { return future; }
  const aliLuaCore::LuaFn       &ExecPoolItem::GetLuaFn () const { return luaFn;  }

}
