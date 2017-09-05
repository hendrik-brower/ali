#include <app2.hpp>
#include <aliLuaCore.hpp>
#include <lua.hpp>
#include <sstream>

namespace {

  struct App2Object {
    using Ptr = std::shared_ptr<App2Object>;
    int    i;
    double j;
    App2Object() : i(0), j(0) {
      INFO("Constructing app2::App2Object " << this);
    }
    ~App2Object() {
      INFO("Deleting app2::App2Object " << this);
    }
    friend std::ostream &operator<<(std::ostream &out, const App2Object &o) {
      return out << "App2Object(i="<<o.i<<", j=" << o.j << ")";
    }
  };
  using OBJ = aliLuaCore::StaticObject<App2Object>;
  
  int Create(lua_State *L) {
    App2Object::Ptr ptr(new App2Object);
    return OBJ::Make(L, ptr);
  }
  int GC(lua_State *L) {
    App2Object::Ptr ptr = OBJ::Get(L, 1, false);
    return 0;
  }
  int ToStr(lua_State *L) {
    App2Object::Ptr ptr = OBJ::Get(L, 1, false);
    std::stringstream ss;
    ss << "App2Object(i=" << ptr->i << ", j=" << ptr->j << ")";
    lua_pushstring(L, ss.str().c_str());
    return 1;
  }
  int IncI(lua_State *L) {
    App2Object::Ptr ptr = OBJ::Get(L, 1, false);
    ++(ptr->i);
    return 0;
  }
  int IncJ(lua_State *L) {
    App2Object::Ptr ptr = OBJ::Get(L, 1, false);
    ptr->j *= 1.34;
    return 0;
  }
  void SerializeObj(lua_State *L,
		    int index,
		    aliSystem::Codec::Serializer &sObj) {
    App2Object::Ptr ptr = OBJ::Get(L,index,false);
    sObj.WriteInt(ptr->i);
    sObj.WriteDouble(ptr->j);
  }
  void DeserializeObj(lua_State *L,
		      aliSystem::Codec::Deserializer &dObj) {
    App2Object::Ptr ptr(new App2Object);
    ptr->i = dObj.ReadInt();
    ptr->j = dObj.ReadDouble();
    OBJ::Make(L, ptr);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("app2 functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("app2 MT");
    mtMap->Add("__gc",       GC);
    mtMap->Add("__tostring", ToStr);
    mtMap->Add("IncI",       IncI);
    mtMap->Add("IncJ",       IncJ);
    OBJ::Init("app2Obj", mtMap, true, SerializeObj, DeserializeObj);
    aliLuaCore::Module::Register("load app2 functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.app2",  fnMap);
				   OBJ::GetMT()->Register(ePtr);
				 });
  }
  void Fini() {
    OBJ::Fini();
  }

}

namespace app2 {
  
  void RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("app2", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }

}

