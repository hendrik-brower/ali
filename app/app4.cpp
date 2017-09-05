#include <app4.hpp>
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>

namespace {

  int SerializeItem(lua_State *L) {
    std::stringstream out;
    aliSystem::Codec::Serializer s(out);
    aliLuaCore::Serialize::Write(L, 1, s);
    std::string str = out.str();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }
  int SerializeItems(lua_State *L) {
    std::stringstream out;
    aliSystem::Codec::Serializer s(out);
    aliLuaCore::Serialize::Write(L, 1, lua_gettop(L), s);
    std::string str = out.str();
    lua_pushlstring(L,  str.data(), str.size());
    return 1;
  }
  int Deserialize(lua_State *L) {
    size_t len;
    const char *str = lua_tolstring(L, 1, &len);
    THROW_IF(!str, "Invalid input");
    std::string in(str, len);
    std::stringstream ss(in);
    aliSystem::Codec::Deserializer d(ss);
    return aliLuaCore::Deserialize::ToLua(L, d);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("app4 functions");
    fnMap->Add("SerializeItem",  SerializeItem);
    fnMap->Add("SerializeItems", SerializeItems);
    fnMap->Add("Deserialize",    Deserialize);
    aliLuaCore::Module::Register("load app4 functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.app4",  fnMap);
				 });
  }
  void Fini() {
  }

}

namespace app4 {
  
  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("app4", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }

}
