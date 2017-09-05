#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_values.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>

namespace aliLuaCore {
  
  MakeTableUtil::MakeTableUtil() {}
  void MakeTableUtil::Set(const MakeFn &key,
			  const MakeFn &value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(key, value));
  }
  void MakeTableUtil::SetBoolean(const std::string &key, bool value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeStringFn(key),
			   Values::GetMakeBoolFn(value)));
  }
  void MakeTableUtil::SetNumber(const std::string &key, int value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeStringFn(key),
			   Values::GetMakeIntegerFn(value)));
  }
  void MakeTableUtil::SetNumber(const std::string &key, double value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeStringFn(key),
			   Values::GetMakeDoubleFn(value)));
  }
  void MakeTableUtil::SetString(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> g(lock);
    if (value.empty()) {
      items.push_back(KVPair(Values::GetMakeStringFn(key),
			     Values::GetMakeEmptyStringFn()));
    } else {
      items.push_back(KVPair(Values::GetMakeStringFn(key),
			     Values::GetMakeStringFn(value)));
    }
  }
  void MakeTableUtil::SetMakeFn(const std::string &key, const MakeFn &value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeStringFn(key), value));
  }
  void MakeTableUtil::SetBooleanForIndex(int key, bool value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeIntegerFn(key),
			   Values::GetMakeBoolFn(value)));
  }
  void MakeTableUtil::SetNumberForIndex(int key, int value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeIntegerFn(key),
			   Values::GetMakeIntegerFn(value)));
  }
  void MakeTableUtil::SetNumberForIndex(int key, double value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeIntegerFn(key),
			   Values::GetMakeDoubleFn(value)));
  }
  void MakeTableUtil::SetStringForIndex(int key, const std::string &value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeIntegerFn(key),
			   Values::GetMakeStringFn(value)));
  }
  void MakeTableUtil::SetMakeFnForIndex(int key, const MakeFn &value) {
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(Values::GetMakeIntegerFn(key), value));
  }
  MakeTableUtil::Ptr MakeTableUtil::CreateSubtable(const std::string &key) {
    return CreateSubtable(Values::GetMakeStringFn(key));
  }
  MakeTableUtil::Ptr MakeTableUtil::CreateSubtableForIndex(int key) {
    return CreateSubtable(Values::GetMakeIntegerFn(key));
  }
  MakeFn MakeTableUtil::GetMakeFn() {
    std::lock_guard<std::mutex> g(lock);
    return [items=this->items](lua_State *dstL) {
      lua_newtable(dstL);
      int tableIndex = lua_gettop(dstL);
      Push(dstL, tableIndex, items);
      return 1;
    };
  }
  int MakeTableUtil::Make(lua_State *L) {
    std::lock_guard<std::mutex> g(lock);
    lua_newtable(L);
    Push(L, lua_gettop(L), items);
    return 1;
  }
  MakeTableUtil::Ptr MakeTableUtil::CreateSubtable(const MakeFn &keyMakeFn) {
    MakeTableUtil::Ptr tblPtr(new MakeTableUtil);
    std::lock_guard<std::mutex> g(lock);
    items.push_back(KVPair(keyMakeFn, [=](lua_State *dstL) -> int {
	  std::lock_guard<std::mutex> g(tblPtr->lock);
	  lua_newtable(dstL);
	  int tableIndex = lua_gettop(dstL);
	  Push(dstL, tableIndex, tblPtr->items);
	  return 1;
	}));
    return tblPtr;
  }
  int MakeTableUtil::Push(lua_State *dstL, int tableIndex, const KVVec &items) {
    tableIndex = lua_absindex(dstL, tableIndex);
    THROW_IF(!lua_istable(dstL, tableIndex), "Expecting a table at " << tableIndex);
    StackGuard g(dstL,2);
    for (KVVec::const_iterator it=items.begin();
	 it!=items.end();
	 ++it) {
      StackGuard g(dstL,2);
      it->first(dstL);
      g.Verify(1, "error pushing key");
      it->second(dstL);
      g.Verify(2, "error pushing value");
      lua_rawset(dstL, tableIndex);
    }
    return 0;
  }

}
