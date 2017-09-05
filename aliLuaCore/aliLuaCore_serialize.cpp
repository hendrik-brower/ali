#include <aliLuaCore_serialize.hpp>
#include <aliLuaCore_functions.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_util.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <limits>
#include <set>

namespace {
  using AddrSet = std::set<const void*>;
  using SObj = aliSystem::Codec::Serializer;
  using SPtr = aliSystem::Codec::Serialize::Ptr;
  
  void SerializeIndex(AddrSet   &addrSet,
		      lua_State *L,
		      int        index,
		      SObj      &sObj);

  void SerializeBool(lua_State *L,
		     int        index,
		     SObj      &sObj) {
    bool val = lua_toboolean(L, index);
    sObj.WriteBool(val);
  }
  void SerializeNumber(lua_State *L,
		       int        index,
		       SObj      &sObj) {
    double dblVal = lua_tonumber(L, index);
    int    intVal = (int)dblVal;
    if (dblVal-intVal==0) {
      sObj.WriteInt(intVal);
    } else {
      sObj.WriteDouble(dblVal);
    }
  }
  void SerializeString(lua_State *L,
		       int        index,
		       SObj      &sObj) {
    const static std::string STR = "STR";
    size_t      len = 0;
    const char *str = lua_tolstring(L, index, &len);
    sObj.WriteString(STR);
    sObj.WriteString(str, len);
  }
  void SerializeTable(lua_State *L,
		      int        index,
		      AddrSet   &addrSet,
		      SObj      &sObj) {
    sObj.WriteString("TBEG");
    lua_checkstack(L,2);
    index = lua_absindex(L, index);
    THROW_IF(!lua_istable(L,index), "expecting table at " << index);
    lua_pushvalue(L, index);
    const void *addr = lua_topointer(L,-1);
    lua_pop(L,1);
    THROW_IF(addrSet.find(addr)!=addrSet.end(),
	     "Serialization of recursive tables is not supported");
    addrSet.insert(addr);
    lua_pushnil(L);
    while (lua_next(L,index)) {
      sObj.WriteString("TKEY");
      SerializeIndex(addrSet, L, -2, sObj);
      SerializeIndex(addrSet, L, -1, sObj);
      lua_pop(L,1);
    }
    sObj.WriteString("TEND");
    addrSet.erase(addr);
  }
  void SerializeUserData(lua_State *L,
			 int        index,
			 SObj      &sObj) {
    std::string name, ud;
    sObj.WriteString("UDBEG");
    aliLuaCore::MT::Serialize(L, index, sObj);
    sObj.WriteString("UDEND");
  }
  void SerializeIndex(AddrSet   &addrSet,
		      lua_State *L,
		      int        index,
		      SObj      &sObj) {
    index = lua_absindex(L,index);
    int type = lua_type(L, index);
    if (false) {
    } else if (type==LUA_TNIL     ) { sObj.WriteString("NIL");
    } else if (type==LUA_TBOOLEAN ) { SerializeBool    (L, index, sObj);
    } else if (type==LUA_TNUMBER  ) { SerializeNumber  (L, index, sObj);
    } else if (type==LUA_TSTRING  ) { SerializeString  (L, index, sObj);
    } else if (type==LUA_TTABLE   ) { SerializeTable   (L, index, addrSet, sObj);
    } else if (type==LUA_TUSERDATA) { SerializeUserData(L, index, sObj);
    } else {
      THROW("Unsupported type: " << lua_typename(L,type) << ", val: " << type);
    }
  }

}
namespace aliLuaCore {


  void Serialize::Write(lua_State *L,
			int        index,
			SObj      &sObj) {
    if (lua_isnone(L,index)) {
    } else {
      AddrSet addrSet;
      SerializeIndex(addrSet, L, index, sObj);
    }
  }
  void Serialize::Write(lua_State *L,
			int        index,
			size_t     count,
			SObj      &sObj) {
    for (size_t i=0; i<count; ++i) {
      if (lua_isnone(L, index+i)) {
	break;
      } else {
	Write(L, index+i, sObj);
      }
    }
  }

}

