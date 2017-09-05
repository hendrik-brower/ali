#include <aliLuaCore_values.hpp>
#include <aliLuaCore_makeTableUtil.hpp>
#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <limits>
#include <string>
#include <unordered_set>


namespace {

  using TableSet = std::unordered_set<const void*>;

  aliLuaCore::MakeFn GetMakeFnForIndex(lua_State *L,
				       int index,
				       TableSet &tableSet);

  int MakeMakeFnVec(lua_State *L, const aliLuaCore::MakeFnVec &mVec) {
    int cnt=0;
    lua_checkstack(L,mVec.size());
    for (aliLuaCore::MakeFnVec::const_iterator it=mVec.begin(); it!=mVec.end(); ++it) {
      aliLuaCore::MakeFn fn = *it;
      cnt += fn(L);
    }
    return cnt;
  }

  
  aliLuaCore::MakeFn GetTableMakeFn(lua_State *srcL, int index, TableSet &tableSet) {
    aliLuaCore::MakeTableUtil mkTblUtil;
    index = lua_absindex(srcL, index);
    THROW_IF(!lua_istable(srcL,index), "Given index " << index << " is not a table");
    lua_checkstack(srcL,2);
    lua_pushnil(srcL);
    while (lua_next(srcL,index)) {
      mkTblUtil.Set(GetMakeFnForIndex(srcL, -2, tableSet),
		    GetMakeFnForIndex(srcL, -1, tableSet));
      lua_pop(srcL,1);
    }
    return mkTblUtil.GetMakeFn();
  }

  aliLuaCore::MakeFn GetMakeFnForIndex(lua_State *L,
				   int index,
				   TableSet &tableSet) {
    if (lua_isnone(L,index)) {
      return aliLuaCore::Values::MakeNothing;
    }
    index = lua_absindex(L, index);
    int type = lua_type(L, index);
    if (false) {
    } else if (type==LUA_TNIL     ) {
      return aliLuaCore::Values::MakeNil;
    } else if (type==LUA_TNUMBER  ) {
      double val = lua_tonumber(L,index);
      return aliLuaCore::Values::GetMakeDoubleFn(val);
    } else if (type==LUA_TSTRING  ) {
      size_t sz = 0;
      const char *cp = lua_tolstring(L, index, &sz);
      if (cp && sz>0) {
	std::string val(cp, sz);
	return aliLuaCore::Values::GetMakeStringFn(val);
      } else {
	return aliLuaCore::Values::MakeEmptyString;
      }
    } else if (type==LUA_TTABLE   ) {
      const void *tableAddr = lua_topointer(L,index);
      std::pair<TableSet::iterator,bool> rtn = tableSet.insert(tableAddr);
      THROW_IF(!rtn.second, "Attempt to build a make function for a recursive table");
      aliLuaCore::MakeFn makeFn = GetTableMakeFn(L, index, tableSet);
      tableSet.erase(rtn.first);
      return makeFn;
    } else if (type==LUA_TUSERDATA) {
      return aliLuaCore::MT::Dup(L,index);
    } else if (type==LUA_TBOOLEAN) {
      bool val = lua_toboolean(L,index);
      if (val) {
	return aliLuaCore::Values::MakeTrue;
      } else {
	return aliLuaCore::Values::MakeFalse;
      }
    } else {
      THROW("Unsupported type: " << type << " " << lua_typename(L,type));
    }
  }

}

namespace aliLuaCore {

  std::string Values::GetString(lua_State *L, int index) {
    index = lua_absindex(L,index);
    std::string rtn; // default to empty string
    size_t len = 0;
    int type = lua_type(L,index);
    switch (type) {
    case LUA_TSTRING: {
      const char *cp = lua_tolstring(L, index, &len);
      if (cp && len>0) {
	rtn = std::string(cp, len);
      }
    } break;
    case LUA_TNUMBER: {
      StackGuard g(L,2);
      lua_pushvalue(L,index);
      const char *cp = lua_tolstring(L,-1, &len);
      rtn = std::string(cp, len);
    } break;
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA: {
      StackGuard g(L,2);
      if (luaL_callmeta(L,index,"__tostring")) {
	if (g.Diff()==1 && lua_isstring(L,-1)) {
	  const char *cp = lua_tolstring(L,-1,&len);
	  rtn = std::string(cp, len);
	}
      }
    } break;
    case LUA_TNIL:
    case LUA_TTHREAD:
    case LUA_TFUNCTION:
    case LUA_TNONE:
    default:
      break;
    }
    return rtn;
  }

    
  // ****************************************************************************************
  // constructors
  int Values::MakeNothing(lua_State *) {
    return 0;
  }
  int Values::MakeNil(lua_State *L) {
    lua_checkstack(L,1);
    lua_pushnil(L);
    return 1;
  }
  int Values::MakeTrue(lua_State *L) {
    lua_checkstack(L,1);
    lua_pushboolean(L, true);
    return 1;
  }
  int Values::MakeFalse(lua_State *L) {
    lua_checkstack(L,1);
    lua_pushboolean(L, false);
    return 1;
  }
  int Values::MakeInteger(lua_State *L, int val) {
    lua_checkstack(L,1);
    lua_pushinteger(L, val);
    return 1;
  }
  int Values::MakeDouble(lua_State *L, double val) {
    lua_checkstack(L,1);
    lua_pushnumber(L, val);
    return 1;
  }
  int Values::MakeString(lua_State *L, const std::string &val) {
    lua_checkstack(L,1);
    lua_pushlstring(L,val.c_str(), val.size());
    return 1;
  }
  int Values::MakeEmptyString(lua_State *L) {
    lua_checkstack(L,1);
    lua_pushlstring(L,"", 0);
    return 1;
  }

  // ****************************************************************************************
  // construction API
  MakeFn Values::GetMakeNothingFn() {
    return [](lua_State *) {
      return 0;
    };
  }
  MakeFn Values::GetMakeNilFn() {
    return [](lua_State *L) {
      return MakeNil(L);
    };
  }
  MakeFn Values::GetMakeBoolFn(bool val) {
    return val ? MakeTrue : MakeFalse;
  }
  MakeFn Values::GetMakeTrueFn() {
    return MakeTrue;
  }
  MakeFn Values::GetMakeFalseFn() {
    return MakeFalse;
  }
  MakeFn Values::GetMakeIntegerFn(int val) {
    return [=](lua_State *L) {
      return MakeInteger(L, val);
    };
  }
  MakeFn Values::GetMakeDoubleFn(double val) {
    return [=](lua_State *L) {
      return MakeDouble(L, val);
    };
  }
  MakeFn Values::GetMakeStringFn(const std::string &val)  {
    return [=](lua_State *L) {
      return MakeString(L, val);
    };
  }
  MakeFn Values::GetMakeEmptyStringFn() {
    return [=](lua_State *L) {
      return MakeEmptyString(L);
    };
  }
  MakeFn Values::GetMakeArrayFn(const MakeFnVec &mVec) {
    return [mVec=mVec](lua_State *L) -> int {
      return MakeMakeFnVec(L,mVec);
    };
  }


  // ****************************************************************************************
  // extraction API
  MakeFn Values::GetMakeFnForAll(lua_State *L) {
    return GetMakeFnByCount(L, 1, std::numeric_limits<size_t>::max());
  }
  MakeFn Values::GetMakeFnForIndex(lua_State *L,
				   int index) {
    TableSet tableSet;
    return ::GetMakeFnForIndex(L,index,tableSet);
  }
  MakeFn Values::GetMakeFnByCount(lua_State *L,
				  int        index,
				  size_t     count) {
    MakeFnVec mVec;
    index = lua_absindex(L,index);
    for (size_t i=0;i<count;++i) {
      if (lua_isnone(L,index+i)) {
	break;
      }
      mVec.push_back(GetMakeFnForIndex(L, index+i));
    }
    return [=](lua_State *L) -> int {
      return MakeMakeFnVec(L, mVec);
    };
  }
  MakeFn Values::GetMakeFnRemaining(lua_State *L, int index) {
    return GetMakeFnByCount(L, index, std::numeric_limits<size_t>::max());
  }


  MakeFn Values::ConstructAsSequence(const MakeFn &makeFn) {
    return [=](lua_State *L) -> int {
      lua_newtable(L);
      int tblIndex = lua_gettop(L);
      int cnt = makeFn(L);
      for (int i=0;i<cnt;++i) {
	lua_seti(L,tblIndex,cnt-i);
      }
      return 1;
    };
  }
    
}

