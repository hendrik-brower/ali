#include <aliLuaCore_deserialize.hpp>
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
#include <ctype.h>

namespace {
  using DObj = aliSystem::Codec::Deserializer;
  using DPtr = aliSystem::Codec::Deserialize::Ptr;

  bool DeserializeNext(lua_State    *L,
		       DObj   &dObj);

  void DeserializeNil(lua_State *L,
		      DObj      &) {
    lua_checkstack(L,1);
    lua_pushnil(L);
  }
  void DeserializeBool(lua_State *L,
		       DObj      &dObj) {
    bool val = dObj.ReadBool();
    lua_checkstack(L,1);
    lua_pushboolean(L, val);
  }
  void DeserializeInt(lua_State *L,
		      DObj      &dObj) {
    lua_checkstack(L,1);
    int val = dObj.ReadInt();
    lua_pushinteger(L, val);
  }
  void DeserializeDouble(lua_State *L,
			 DObj      &dObj) {
    lua_checkstack(L,1);
    double val = dObj.ReadDouble();
    lua_pushnumber(L, val);
  }
  void DeserializeString(lua_State *L,
			 DObj      &dObj) {
    lua_checkstack(L,1);
    std::string str;
    dObj.ReadString(str);
    lua_pushlstring(L, str.c_str(), str.size());
  }

  void DeserializeTable(lua_State *L,
			DObj      &dObj) {
    lua_checkstack(L,3);
    lua_newtable(L);
    while (true) {
      std::string type;
      dObj.ReadString(type);
      if (type=="TEND") {
	break;
      }
      if (type=="TKEY") {
	THROW_IF(!DeserializeNext(L, dObj), "failed to extrat a table key");
	THROW_IF(!DeserializeNext(L, dObj), "Failed to extract a table value");
	lua_rawset(L, -3);
      } else {
	THROW("Unexpected type: " << type);
      }
    }
  }
  void DeserializeUserData(lua_State *L,
			   DObj      &dObj) {
    aliLuaCore::MT::Deserialize(L, dObj);
    std::string udTerm;
    dObj.ReadString(udTerm);
    THROW_IF(udTerm!="UDEND", "User data terminator not found, got: " << udTerm);
  }
  bool DeserializeNext(lua_State *L,
		       DObj      &dObj) {
    using Type = aliSystem::Codec::Deserialize::Type;
    Type next = dObj.NextType();
    switch (next) {
    case Type::END:
      return false;
    case Type::INVALID:
      THROW("Invalid input");
    case Type::BOOL:
      DeserializeBool(L,dObj);
      break;
    case Type::INT:
      DeserializeInt(L,dObj);
      break;
    case Type::DOUBLE:
      DeserializeDouble(L,dObj);
      break;
    case Type::STRING: {
      std::string val;
      dObj.ReadString(val);
      if (false) {
      } else if (val=="TBEG") {
	DeserializeTable(L,dObj);
      } else if (val=="UDBEG") {
	DeserializeUserData(L,dObj);
      } else if (val=="NIL") {
	DeserializeNil(L,dObj);
      } else if (val=="STR") {
	DeserializeString(L,dObj);
      } else {
	THROW("Unrecognized type string val=" << val);
      }
      break;
    }
    default:
      THROW("Unrecognized input");
    }
    return true;
  }
}
namespace aliLuaCore {
  
  int Deserialize::ToLua(lua_State *L,
			 DObj      &dObj) {
    int top = lua_gettop(L);
    while (dObj.IsGood()
	   && !dObj.IsEOF()
	   && DeserializeNext(L, dObj)) {
    }
    return lua_gettop(L)-top;
  }

  MakeFn Deserialize::GetMakeFn(DObj &dObj) {
    std::string inStr;
    dObj.ReadAll(inStr);
    DPtr dPtr = dObj.GetDeserialize();
    return [=](lua_State *L) -> int {
      std::istringstream in(inStr);
      DObj obj(in, dPtr);
      return ToLua(L, obj);
    };
  }

}

