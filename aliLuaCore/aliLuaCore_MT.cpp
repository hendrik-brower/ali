#include <aliLuaCore_MT.hpp>
#include <aliLuaCore_deserialize.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_module.hpp>
#include <aliLuaCore_util.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_serialize.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <cstring>

namespace {
  using MTMap = std::map<std::string, aliLuaCore::MT::WPtr>;
  std::mutex  lock;
  MTMap       mtMap;
  const char *MT_OBJ_KEY = "__mtObjKey";

  void Add(const std::string &name, const aliLuaCore::MT::Ptr &ptr) {
    THROW_IF(!ptr, "Undefined MT for " << name);
    std::lock_guard<std::mutex> g(lock);
    MTMap::const_iterator it = mtMap.find(name);
    THROW_IF(it!=mtMap.end(), "MT already defined for: " << name);
    mtMap[name] = ptr;
  }
  aliLuaCore::MT::Ptr GetMT(const std::string &name) {
    aliLuaCore::MT::Ptr rtn;
    std::lock_guard<std::mutex> g(lock);
    MTMap::const_iterator it = mtMap.find(name);
    if (it!=mtMap.end()) {
      rtn = it->second.lock();
    }
    return rtn;
  }

  aliLuaCore::MakeFn Dup(lua_State *srcL, int index) {
    lua_checkstack(srcL, 1);
    aliLuaCore::MT::Ptr mtPtr  = aliLuaCore::MT::GetMT(srcL, index, false);
    int                 vType  = lua_getuservalue(srcL,index);
    THROW_IF(vType!=LUA_TBOOLEAN, "Expecting user value to be boolean");
    bool         isWeak = lua_toboolean(srcL,-1);
    lua_pop(srcL,1);
    if (isWeak) {
      aliLuaCore::MT::WTPtr *wtPtrPtr = (aliLuaCore::MT::WTPtr*)lua_touserdata(srcL,index);
      THROW_IF(!wtPtrPtr, "Object has been released");
      aliLuaCore::MT::WTPtr wtPtr = *wtPtrPtr;
      return [=](lua_State *dstL) -> int {
	mtPtr->MakeWeakObject(dstL, wtPtr);
	return 1;
      };
    } else {
      aliLuaCore::MT::TPtr *tPtrPtr = (aliLuaCore::MT::TPtr*)lua_touserdata(srcL, index);
      THROW_IF(!tPtrPtr, "Object has been released");
      aliLuaCore::MT::TPtr tPtr = *tPtrPtr;
      return [=](lua_State *dstL) -> int {
	mtPtr->MakeObject(dstL, tPtr);
	return 1;
      };
    }
  }
  int IsValid(lua_State *L) {
    aliLuaCore::MT::Ptr  mtPtr = aliLuaCore::MT::GetMT(L, 1, false);
    aliLuaCore::MT::TPtr tPtr  = mtPtr->Get(L,1,true);
    lua_checkstack(L,1);
    lua_pushboolean(L,!!tPtr);
    return 1;
  }
  aliLuaCore::MakeFn NoDup(lua_State *, int) {
    THROW("duplication is not supported for this object");
  }
  int GC(const aliLuaCore::LuaFn &orig, lua_State *L) {
    static const char nullPointer[] = { 0, 0, 0, 0, 0, 0, 0, 0 }; 
    int index = 1;
    lua_checkstack(L,1);
    int vType = lua_getuservalue(L,index);
    THROW_IF(vType!=LUA_TBOOLEAN, "Expecting user value to be boolean");
    bool isWeak = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (isWeak) {
      aliLuaCore::MT::WTPtr *wtPtr = (aliLuaCore::MT::WTPtr*)lua_touserdata(L, index);
      if (std::memcmp(wtPtr,nullPointer, sizeof(aliLuaCore::MT::WTPtr))) {
	// is there a cleaner technique than memcpy?
	if (orig && !wtPtr->expired()) {
	  orig(L);
	}
	wtPtr->reset();
	using WTPTR = aliLuaCore::MT::WTPtr;
	wtPtr->~WTPTR(); // this zero's the memory, is that by spec, or just g++?
      }
    } else {
      aliLuaCore::MT::TPtr *tPtr = (aliLuaCore::MT::TPtr*)lua_touserdata(L, index);
      if (std::memcmp(tPtr, nullPointer, sizeof(aliLuaCore::MT::TPtr))) { // is there a cleaner technique?
	if (orig && *tPtr) {
	  orig(L);
	}
	tPtr->reset();
	using TPTR = aliLuaCore::MT::TPtr;
	tPtr->~TPTR(); // this zero's the memory, is that by spec, or just g++?
      }
    }
    return 0;
  }
  int ToString(const std::string &mtName, const aliLuaCore::LuaFn &orig, lua_State *L) {
    aliLuaCore::MT::Ptr ptr = aliLuaCore::MT::GetMT(L, 1, false);
    THROW_IF(ptr->Name()!=mtName, "Invalid object reference");
    aliLuaCore::MT::TPtr tPtr = ptr->Get(L, 1, true);
    if (tPtr && orig) {
      return orig(L);
    } else {
      std::string str = mtName;
      if (!tPtr) { str += "<released>"; }
      lua_pushstring(L, str.c_str());
      return 1;
    }
  }

  int GetMetatables(lua_State *) {
    return 0;
  }
  int ToWeak(lua_State *L) {
    int index = 1;
    aliLuaCore::MT::Ptr  ptr = aliLuaCore::MT::GetMT(L,index,false);
    return ptr->ToWeak(L, index);
  }
  int ToStrong(lua_State *L) {
    int index = 1;
    aliLuaCore::MT::Ptr  ptr = aliLuaCore::MT::GetMT(L,index,false);
    return ptr->ToStrong(L, index);
  }

  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("MT functions");
    fnMap->Add("GetMetatables", GetMetatables);
    fnMap->Add("ToWeak",        ToWeak);
    fnMap->Add("ToStrong",      ToStrong);
    aliLuaCore::Module::Register("load aliLuaCore::MT functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.mt",  fnMap);
			     });
  }
  void Fini() {}
  
}


namespace aliLuaCore {

  void MT::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaCore::MT", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore::Module");
  }
  
  MT::Ptr MT::Create(const std::string      &name,
		     const FunctionMap::Ptr &fnMap,
		     bool                    allowDup) {
    SerializeFn   serializeFn;
    DeserializeFn deserializeFn;
    Ptr rtn = Create(name, fnMap, allowDup, serializeFn, deserializeFn);
    rtn->canSerialize = false;
    return rtn;
  }
  MT::Ptr MT::Create(const std::string      &name,
		     const FunctionMap::Ptr &fnMap,
		     bool                    allowDup,
		     SerializeFn             serializeFn,
		     DeserializeFn           deserializeFn) {
    THROW_IF(!fnMap, "Function Map when attempting to create " << name);
    Ptr rtn(new MT);
    rtn->THIS          = rtn;
    rtn->name          = name;
    rtn->fnMap         = fnMap;
    rtn->dupFn         = allowDup ? ::Dup : ::NoDup;
    rtn->serializeFn   = serializeFn;
    rtn->deserializeFn = deserializeFn;
    rtn->canDup        = allowDup;
    rtn->canSerialize  = true;
    DefineReserved(rtn);
    Add(name, rtn);
    fnMap->Freeze();
    return rtn;
  }
  MT::~MT() {}
  const std::string &MT::Name() const { return name; }
  MT::Ptr MT::GetMT(lua_State *L, int index, bool ignoreNonMT) {
    Ptr               rtn;
    StackGuard  g(L,1);
    int rc = luaL_getmetafield(L, index, MT_OBJ_KEY);
    THROW_IF(rc==0 && !ignoreNonMT, "Unrecognized object");
    if (rc) {
      Ptr *p = (Ptr*)lua_touserdata(L,-1);
      THROW_IF(!p, "Metatable is null");
      rtn = *p;
      THROW_IF(!p, "Null metatable pointer");
    }
    return rtn;
  }
  MT::Ptr MT::GetMT(const std::string &name) {
    return ::GetMT(name);
  }
  bool MT::Is(lua_State *L, int index) {
    Ptr p = GetMT(L, index, true);
    return p.get()==this;
  }
  MT::TPtr MT::Get(lua_State *L, int index, bool allowNull) {
    TPtr rtn;
    if (allowNull && (lua_isnil(L,index) || lua_isnone(L,index))) {
    } else {
      lua_checkstack(L,1);
      bool is = Is(L,index);
      if (!is) {
	std::lock_guard<std::mutex> g(derivedLock);
	for (WPVec::iterator it=derivedVec.begin(); it!=derivedVec.end(); ++it) {
	  Ptr mtPtr = it->lock();
	  if (mtPtr) {
	    if (mtPtr->Is(L,index)) {
	      return mtPtr->Get(L,index,allowNull);
	    }
	  }
	}
      }
      THROW_IF(!is, "invalid conversion to " << name);
      int vType = lua_getuservalue(L,index);
      THROW_IF(vType!=LUA_TBOOLEAN, "Expecting user value to be boolean");
      bool isWeak = lua_toboolean(L,-1);
      lua_pop(L,1);
      if (isWeak) {
	WTPtr *wtPtr = (WTPtr*)lua_touserdata(L,index);
	THROW_IF(!wtPtr && !allowNull, "Object has been released");
	rtn = wtPtr ? wtPtr->lock() : rtn;
      } else {
	TPtr *tPtr = (TPtr*)lua_touserdata(L, index);
	THROW_IF(!tPtr && !allowNull, "Object has been released");
	rtn = tPtr ? *tPtr : rtn;
      }
      THROW_IF(!rtn && !allowNull, "Object has been released");
    }
    return rtn;
  }
  int MT::MakeObject(lua_State *L, const TPtr &tPtr) {
    lua_checkstack(L,2);
    new (lua_newuserdata(L,sizeof(TPtr))) TPtr(tPtr);
    lua_pushboolean(L, false); // flag as not weak
    lua_setuservalue(L, -2);
    SetMT(L);
    return 1;
  }
  int MT::MakeWeakObject(lua_State *L, const WTPtr &wtPtr) {
    lua_checkstack(L,2);
    new (lua_newuserdata(L, sizeof(WTPtr))) WTPtr(wtPtr);
    lua_pushboolean(L, true); // flag as weak
    lua_setuservalue(L,-2);
    SetMT(L);
    return 1;
  }
  int MT::ToWeak(lua_State *L, int index) {
    THROW_IF(!Is(L,index), "Invalid type");
    TPtr rtn;
    lua_checkstack(L,1);
    int vType = lua_getuservalue(L,index);
    THROW_IF(vType!=LUA_TBOOLEAN, "Expecting user value to be boolean");
    bool isWeak = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (isWeak) {
      lua_pushvalue(L,index);
    } else {
      TPtr *tPtr = (TPtr*)lua_touserdata(L, index);
      THROW_IF(!tPtr, "Object has been released");
      WTPtr wtPtr = *tPtr;
      MakeWeakObject(L, wtPtr);
    }
    return 1;
  }
  int MT::ToStrong(lua_State *L, int index) {
    TPtr rtn;
    THROW_IF(!Is(L,index), "Invalid type");
    lua_checkstack(L,1);
    int vType = lua_getuservalue(L,index);
    THROW_IF(vType!=LUA_TBOOLEAN, "Expecting user value to be boolean");
    bool isWeak = lua_toboolean(L,-1);
    lua_pop(L,1);
    if (isWeak) {
      WTPtr *wtPtr = (WTPtr*)lua_touserdata(L, index);
      THROW_IF(!wtPtr, "Object has been released");
      TPtr tPtr = wtPtr->lock();
      MakeObject(L, tPtr);
    } else {
      lua_pushvalue(L,index);
    }
    return 1;
  }
  MakeFn MT::Dup(lua_State *L, int index) {
    Ptr ptr = GetMT(L, index, false);
    THROW_IF(!ptr->dupFn, "Unable to duplicate the object");
    return ptr->dupFn(L,index);
  }
  void MT::Serialize(lua_State *L,
		     int        index,
		     SObj      &sObj) {
    Ptr ptr = GetMT(L, index, false);
    THROW_IF(!ptr->serializeFn, "Unable to serialize the object");
    sObj.WriteString(ptr->name);
    std::ostringstream objOut;
    SObj obj(objOut,sObj.GetSerialize());
    ptr->serializeFn(L, index, obj);
    sObj.WriteString(objOut.str());
  }
  void MT::Deserialize(lua_State *L,
		       DObj      &dObj) {
    std::string name,ud;
    dObj.ReadString(name);
    dObj.ReadString(ud);
    aliLuaCore::MT::Ptr    ptr = aliLuaCore::MT::GetMT(name);
    THROW_IF(!ptr->deserializeFn, "Unable to deserialize the object");
    std::istringstream udIn(ud);
    DObj obj(udIn, dObj.GetDeserialize());
    ptr->deserializeFn(L, obj);
  }
  void MT::Register(const Exec::Ptr &ePtr) {
    Util::Run(ePtr, [=](lua_State *L) -> int {
	aliLuaCore::StackGuard g(L,2);
	if (luaL_newmetatable(L, name.c_str())) {
	  ePtr->Registry()->Register(THIS.lock());
	  lua_pushvalue(L,-1);
	  lua_setfield(L,-2, "__index");
	  lua_pushlstring(L, name.c_str(), name.size());
	  lua_setfield(L,-2, "__metatable");
	  lua_pushlightuserdata(L, THIS.lock().get());
	  lua_setfield(L,-2, MT_OBJ_KEY);
	  std::string baseName = "MT:";
	  baseName.append(name+".");
	  Util::LoadFnMap(ePtr->Registry(), L, -1, baseName, fnMap);
	} else {
	  THROW("Failed to create meta table: " << name);
	}
	return 0;
      });
  }
  bool MT::CanDup() const {
    return canDup;
  }
  bool MT::CanSerialize() const {
    return canSerialize;
  }
  bool MT::IsDefined(const std::string &fnName) {
    return fnMap->IsDefined(fnName);
  }
  void MT::AddDerived(const Ptr &mtPtr) {
    THROW_IF(!mtPtr, "mtPtr is null");
    std::lock_guard<std::mutex> g(derivedLock);
    derivedVec.push_back(mtPtr);
  }
  void MT::SetMT(lua_State *L) const {
    luaL_setmetatable(L,name.c_str());
  }
  void MT::DefineReserved(const Ptr &ptr) {
    const std::string name = ptr->name;
    ptr->fnMap->Wrap("__gc",       GC);
    ptr->fnMap->Wrap("__tostring", [=](const aliLuaCore::LuaFn &orig, lua_State *L)->int {
	return ToString(name, orig, L);
      });
    ptr->fnMap->Add("ToWeak",  [=](lua_State *L) -> int {
	THROW_IF(!ptr->Is(L,1), "unrecognized object");
	return ptr->ToWeak(L,1);
      });
    ptr->fnMap->Add("ToStrong", [=](lua_State *L) -> int {
	THROW_IF(!ptr->Is(L,1), "unrecognized object");
	return ptr->ToStrong(L,1);
      });
    ptr->fnMap->Add("IsValid",  [=](lua_State *L) -> int {
	return IsValid(L);
      });
    ptr->fnMap->Add("MTName",  [=](lua_State *L) -> int {
	lua_pushstring(L,ptr->Name().c_str());
	return 1;
      });
    ptr->fnMap->Alias("Release", "__gc");
  }

  MT::MT() {}

}

