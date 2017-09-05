#include <aliLuaExt_codec.hpp>

namespace {
  using DOBJ = aliLuaCore::StaticObject<aliSystem::Codec::Deserialize>;
  using SOBJ = aliLuaCore::StaticObject<aliSystem::Codec::  Serialize>;

  int GetBasicSerialize(lua_State *L) {
    return SOBJ::Make(L, aliSystem::BasicCodec::GetSerializer());
  }
  int GetBasicDeserialize(lua_State *L) {
    return DOBJ::Make(L, aliSystem::BasicCodec::GetDeserializer());
  }
  int GetSInfo(lua_State *L) {
    SOBJ::TPtr ptr = SOBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil mtu;
    mtu.SetString("name",    ptr->Name());
    mtu.SetNumber("version", (int)ptr->Version());
    return mtu.Make(L);
  }
  int GetDInfo(lua_State *L) {
    DOBJ::TPtr ptr = DOBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil mtu;
    mtu.SetString("name",    ptr->Name());
    mtu.SetNumber("version", (int)ptr->Version());
    return mtu.Make(L);
  }
  int CanDeserialize(lua_State *L) {
    DOBJ::TPtr ptr      = DOBJ::Get(L,1,false);
    std::string name    = aliLuaCore::Values::GetString(L,2);
    int         version = lua_tointeger(L,3);
    if (ptr->CanDeserialize(name, (size_t)version)) {
      aliLuaCore::Values::MakeTrue(L);
    } else {
      aliLuaCore::Values::MakeFalse(L);
    }
    return 1;
  }
  int Serialize(lua_State *L) {
    std::stringstream out;
    SOBJ::TPtr ptr = SOBJ::Get(L,1,false);
    aliSystem::Codec::Serializer s(out, ptr);
    aliLuaCore::Serialize::Write(L, 2, lua_gettop(L), s);
    return aliLuaCore::Values::MakeString(L,out.str());
  }
  int Deserialize(lua_State *L) {
    DOBJ::TPtr        ptr = DOBJ::Get(L,1,false);
    std::string       str = aliLuaCore::Values::GetString(L, 2);
    std::stringstream in(str);
    aliSystem::Codec::Deserializer d(in, ptr);
    return aliLuaCore::Deserialize::ToLua(L, d);
  }
  
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("codec functions");
    fnMap->Add("GetBasicSerialize",   GetBasicSerialize);
    fnMap->Add("GetBasicDeserialize", GetBasicDeserialize);
    aliLuaCore::FunctionMap::Ptr sMTMap = aliLuaCore::FunctionMap::Create("serialize");
    aliLuaCore::FunctionMap::Ptr dMTMap = aliLuaCore::FunctionMap::Create("deserialize");
    sMTMap->Add("GetInfo", GetSInfo);
    dMTMap->Add("GetInfo", GetDInfo);
    dMTMap->Add("CanDeserialize", CanDeserialize);
    sMTMap->Add("Serialize",   Serialize);
    dMTMap->Add("Deserialize", Deserialize);
    SOBJ::Init("serialize",   sMTMap, true);
    DOBJ::Init("deserialize", dMTMap, true);
    aliLuaCore::Module::Register("load aliLuaCore::Call functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr,
							       "lib.aliLua.codec",
							       fnMap);
				   SOBJ::Register(ePtr);
				   DOBJ::Register(ePtr);
				 });
  }
  void Fini() {
    SOBJ::Fini();
    DOBJ::Fini();
  }
}

namespace aliLuaExt {

  void Codec::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::Codec", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  
}
