#include <aliLuaCore_table.hpp>
#include <aliLuaCore_deserialize.hpp>
#include <aliLuaCore_serialize.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_values.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>


namespace aliLuaCore {

  bool Table::IsNil(lua_State *L,
		    int tableIndex,
		    const std::string &key) {
    bool rtn = false;
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    rtn = lua_isnil(L,-1);
    return rtn;
  }
  
  void Table::SetNil(lua_State *L,
		     int tableIndex,
		     const std::string &key) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    lua_pushnil(L);
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetInteger(lua_State *L,
			 int tableIndex,
			 const std::string &key,
			 int &value,
			 bool allowNil,
			 int defaultValue) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      THROW_IF(!allowNil, "nil value for " << key);
      value = defaultValue;
    } else {
      value = lua_tointeger(L, -1);
    }
  }
  
  void Table::SetInteger(lua_State *L,
			 int tableIndex,
			 const std::string &key,
			 int  value) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_pushinteger(L,value);
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetDouble(lua_State *L,
			int tableIndex,
			const std::string &key,
			double &value,
			bool allowNil,
			double defaultValue) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      THROW_IF(!allowNil, "nil value for " << key);
      value = defaultValue;
    } else {
      value = lua_tonumber(L, -1);
    }
  }
  
  void Table::SetDouble(lua_State *L,
			int tableIndex,
			const std::string &key,
			double value) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_pushnumber(L,value);
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetString(lua_State *L,
			int tableIndex,
			const std::string &key,
			std::string &value,
			bool allowNil,
			const std::string &defaultValue) {
    value.clear();
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      THROW_IF(!allowNil, "nil value for " << key);
      value = defaultValue;
    } else {
      value = Values::GetString(L,-1);
    }
  }
  
  void Table::SetString(lua_State *L,
			int tableIndex,
			const std::string &key,
			const std::string &value) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    Values::MakeString(L,value);
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetBool(lua_State *L,
		      int tableIndex,
		      const std::string &key,
		      bool &value,
		      bool allowNil,
		      bool defaultValue) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L,tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      THROW_IF(!allowNil, "nil value for " << key);
      value = defaultValue;
    } else {
      value = lua_toboolean(L,-1);
    }
  }
  
  void Table::SetBool(lua_State *L,
		      int tableIndex,
		      const std::string &key,
		      bool value) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_pushboolean(L,value);
    lua_setfield(L,tableIndex, key.c_str());
  }
  
  void Table::GetMakeFn(lua_State *L,
			int tableIndex,
			const std::string &key,
			MakeFn &makeFn,
			bool allowNil,
			const MakeFn &defaultValue) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L,tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      THROW_IF(!allowNil, "nil value for " << key);
      makeFn = defaultValue;
    } else {
      makeFn = Values::GetMakeFnForIndex(L,-1);
    }
  }
  
  void Table::SetMakeFn(lua_State *L,
			int tableIndex,
			const std::string &key,
			const MakeFn &makeFn) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    makeFn(L);
    if (g.Diff()==0) {
      lua_pushnil(L);
    }
    THROW_IF(g.Diff()!=1, "MakeFn should have pushed a single value instead of " << g.Diff());
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetSerializedValue(lua_State         *L,
				 int                tableIndex,
				 const std::string &key,
				 SObj              &sObj,
				 bool               allowNil) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    THROW_IF(!allowNil && lua_isnil(L,-1), "nil value for key " << key);
    Serialize::Write(L, -1, sObj);
  }
  
  void Table::SetSerializedValue(lua_State         *L,
				 int                tableIndex,
				 const std::string &key,
				 DObj              &dObj) {
    StackGuard g(L,1);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    Deserialize::ToLua(L, dObj);
    if (g.Diff()==0) {
      // serialized string contained no value
      lua_pushnil(L);
    }
    THROW_IF(g.Diff()!=1, "Deserialize should have pushed a single value instead of " << g.Diff());
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetSequence(lua_State *L,
			  int tableIndex,
			  const std::string &key,
			  MakeFn &makeFn,
			  bool allowNil) {
    StackGuard g(L,2);
    THROW_IF(!lua_istable(L,tableIndex), "expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L,tableIndex, key.c_str());
    if (allowNil && lua_isnil(L,-1)) {
      makeFn = Values::MakeNothing;
    } else {
      THROW_IF(!lua_istable(L,-1), "Expecting a table for the key " << key);
      MakeFnVec mVec;
      for (int i=1;;++i) {
	int type = lua_geti(L,-1, i);
	if (type==LUA_TNIL) {
	  break;
	}
	mVec.push_back(Values::GetMakeFnForIndex(L,-1));
	lua_pop(L,1);
      }
      makeFn = Values::GetMakeArrayFn(mVec);
    }
  }
  
  void Table::GetStringSequence(lua_State *L,
				int tableIndex,
				const std::string &key,
				SSet &sSet) {
    StackGuard g(L,2);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      // no data
    } else {
      THROW_IF(!lua_istable(L,-1), "Expecting " << key << " to be a table");
      int index = 0;
      while (true) {
	StackGuard g(L,1);
	lua_geti(L,-1,++index);
	if (lua_isnil(L,-1)) { break; }
	std::string str = Values::GetString(L,-1);
	sSet.insert(str);
      }
    }
  }
  
  void Table::SetStringSequence(lua_State *L,
				int tableIndex,
				const std::string &key,
				const SSet &sSet) {
    int i = 0;
    StackGuard g(L,3);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_createtable(L, sSet.size(), 0);
    for (SSet::const_iterator it=sSet.begin(); it!=sSet.end(); ++it) {
      lua_pushlstring(L,it->c_str(), it->size());
      lua_seti(L,-2,++i);
    }
    lua_setfield(L, tableIndex, key.c_str());
  }
  
  void Table::GetStringSequence(lua_State *L,
				int tableIndex,
				const std::string &key,
				SVec &sVec) {
    StackGuard g(L,2);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_getfield(L, tableIndex, key.c_str());
    if (lua_isnil(L,-1)) {
      // no data
    } else {
      THROW_IF(!lua_istable(L,-1), "Expecting " << key << " to be a table");
      int index = 0;
      while (true) {
	StackGuard g(L,1);
	lua_geti(L,-1,++index);
	if (lua_isnil(L,-1)) { break; }
	sVec.push_back(Values::GetString(L,-1));
      }
    }
  }
  
  void Table::SetStringSequence(lua_State *L,
				int tableIndex,
				const std::string &key,
				const SVec &sVec) {
    StackGuard g(L,3);
    THROW_IF(!lua_istable(L,tableIndex), "Expecting a table");
    tableIndex = lua_absindex(L,tableIndex);
    lua_createtable(L, sVec.size(), 0);
    for (SVec::const_iterator it=sVec.begin(); it!=sVec.end(); ++it) {
      lua_pushlstring(L,it->c_str(), it->size());
      lua_seti(L,-2,it-sVec.begin()+1);
    }
    lua_setfield(L, tableIndex, key.c_str());
  }

}
