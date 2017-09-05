#include <aliLuaCore_util.hpp>
#include <aliLuaCore_exec.hpp>
#include <aliLuaCore_future.hpp>
#include <aliLuaCore_stackGuard.hpp>
#include <aliLuaCore_values.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <algorithm>

namespace {
  const int UP_FN_OBJECT_PTR = 1;
  const int UP_LUA_FUNCTOR   = 1;

  struct FnObject {
    using Ptr = std::shared_ptr<FnObject>;
    FnObject(const std::string &fnName_,
	     const aliLuaCore::LuaFn  &fn_)
      : fn(fn_),
	stats(new aliSystem::Stats(fnName_)) {
    }
    ~FnObject() {}
    int Fn(lua_State *L) { return fn(L); }
    const aliSystem::Stats::Ptr &Stats() { return stats; }
  private:
    aliLuaCore::LuaFn     fn;
    aliSystem::Stats::Ptr stats;
  };

  std::string FirstLine(const std::string &str) {
    size_t pos = str.find("\n");
    if (std::string::npos==pos) {
      pos = str.size();
    }
    if (pos>25) {
      pos = 25;
    }
    return std::string(str, 0, pos);
  }
  
  int HandleFn(lua_State *L) {
    FnObject *p = (FnObject*)lua_touserdata(L, lua_upvalueindex(UP_FN_OBJECT_PTR));
    if (p) {
      try {
	aliSystem::StatsGuard sg(p->Stats());
	return p->Fn(L);
      } catch (std::exception &e) {
	lua_pushstring(L, e.what());
      }
    } else {
      lua_pushstring(L, "failed to retrieve function");
    }
    return lua_error(L);
  }

  int RunLuaFn(lua_State *L) {
    int  fnIndex = lua_upvalueindex(UP_LUA_FUNCTOR);
    const aliLuaCore::LuaFn *fn = (const aliLuaCore::LuaFn*)lua_topointer(L,fnIndex);
    try {
      return (*fn)(L);
    } catch (std::exception &e) {
      lua_pushstring(L, e.what());
    }
    return lua_error(L);
  }

  //  LoadTable: swap table at tblIdx, with a sub table defined by the period separated path
  void LoadTable(lua_State         *L,
		 int                tblIdx,
		 const std::string &path) {
    aliLuaCore::StackGuard g(L,2);
    THROW_IF(lua_type(L,tblIdx)!=LUA_TTABLE, "Attempt to load a table to a non-table element");
    if (!path.empty()) {
      size_t pos = path.find('.');
      if (pos!=std::string::npos) {
	std::string a = path.substr(0,pos);
	std::string b = path.substr(pos+1);
	LoadTable(L, tblIdx, a);
	LoadTable(L, tblIdx, b);
      } else {
	lua_getfield(L, tblIdx, path.c_str());
	if (lua_type(L,-1)!=LUA_TTABLE) {
	  // non-table, overwrite
	  lua_pop(L,1);
	  lua_newtable(L);
	  lua_pushvalue(L,-1);
	  lua_setfield(L, tblIdx, path.c_str());
	} else if (lua_type(L,-1)==LUA_TTABLE) {
	  // use existing table
	} else {
	  THROW("Invalid path");
	}
	lua_replace(L, tblIdx);
      }
    }
  }

  //
  // LoadFn.
  //   Load the global function identified by fnName using the GetElement
  //   interface to extract the value.
  void LoadFn(lua_State         *L,
	      const std::string &fnName) {
    int type = aliLuaCore::Util::GetElement(L,fnName);
    THROW_IF(type!=LUA_TFUNCTION, "Expecting a function");
  }

}

namespace aliLuaCore {
    
  std::string Util::RCStr(int rc) {
    std::string rtn = "<unknown>";
    if (false) {
    } else if (rc==LUA_OK       ) { rtn = "LUA_OK";
    } else if (rc==LUA_ERRRUN   ) { rtn = "LUA_ERRRUN";
    } else if (rc==LUA_ERRMEM   ) { rtn = "LUA_ERRMEM";
    } else if (rc==LUA_ERRERR   ) { rtn = "LUA_ERRERR";
    } else if (rc==LUA_ERRGCMM  ) { rtn = "LUA_ERRGCMM";
    } else if (rc==LUA_ERRFILE  ) { rtn = "LUA_ERRFILE";
    } else if (rc==LUA_ERRSYNTAX) { rtn = "LUA_ERRSYNTAX";
    } else if (rc==LUA_YIELD    ) { rtn = "LUA_YIELD";
    } else {
      char buf[25];
      snprintf(buf, sizeof(buf), "unknown(%i)", rc);
      rtn = buf;
    }
    return rtn;
  }

  void Util::Run(const Exec::Ptr &ePtr,
		 const LuaFn     &fn) {
    Future::Ptr result;
    Run(ePtr, result, fn);
  }
  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const std::string &fnName) {
    Future::Ptr result;
    RunFn(ePtr, result, fnName);
  }
  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const std::string &fnName,
		   const SVec &args) {
    Future::Ptr result;
    RunFn(ePtr, result, fnName, args);
  }
  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const std::string &fnName,
		   const MakeFn      &args) {
    Future::Ptr result;
    RunFn(ePtr, result, fnName, args);
  }
  void Util::RunMT(const Exec::Ptr   &ePtr,
		   const MakeFn      &obj,
		   const std::string &fnName,
		   const MakeFn      &args) {
    Future::Ptr result;
    RunMT(ePtr, result, obj, fnName, args);
  }
  void Util::LoadFile(const Exec::Ptr   &ePtr,
		      const std::string &path) {
    Future::Ptr result;
    LoadFile(ePtr, result, path);
  }
  void Util::LoadString(const Exec::Ptr   &ePtr,
			const std::string &str) {
    Future::Ptr result;
    LoadString(ePtr, result, str);
  }
  void Util::LoadFnMap(const Exec::Ptr        &ePtr,
		       const std::string      &path,
		       const FunctionMap::Ptr &fnMap) {
    Future::Ptr result;
    fnMap->Freeze();
    LoadFnMap(ePtr, result, path, fnMap);
  }

  void Util::Run(const Exec::Ptr   &ePtr,
		 const Future::Ptr &future,
		 const LuaFn       &fn) {
    THROW_IF(!ePtr, "Call to Run with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	lua_pushlightuserdata(L, (void*)&fn); // UP_LUA_FUNCTOR
	lua_pushcclosure(L,RunLuaFn,1);
	int rc = lua_pcall(L,0,LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK,
		 "Error running function, rc " << rc
		 << ", err " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }

  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const Future::Ptr &future,
		   const std::string &fnName) {
    SVec args;      
    RunFn(ePtr, future, fnName, args);
  }

  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const Future::Ptr &future,
		   const std::string &fnName,
		   const SVec        &args) {
    THROW_IF(!ePtr, "Call to RunFn with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	LoadFn(L, fnName);
	for (SVec::const_iterator it=args.begin(); it!=args.end(); ++it) {
	  lua_pushlstring(L, it->c_str(), it->size());
	}
	int rc = lua_pcall(L, args.size(), LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK,
		 "Error running " << fnName << ", rc " << rc
		 << ", err " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }
  void Util::RunFn(const Exec::Ptr   &ePtr,
		   const Future::Ptr &future,
		   const std::string &fnName,
		   const MakeFn      &args) {
    THROW_IF(!ePtr, "Call to RunFn with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	LoadFn(L, fnName);
	int numArgs = args(L);
	int rc      = lua_pcall(L, numArgs, LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK,
		 "Error running " << fnName << ", rc " << rc
		 << ", err " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }
  void Util::RunMT(const Exec::Ptr   &ePtr,
		   const Future::Ptr &future,
		   const MakeFn      &obj,
		   const std::string &fnName,
		   const MakeFn      &args) {
    THROW_IF(!ePtr, "Call to RunMT with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	int top = lua_gettop(L);
	obj(L); // create object
	THROW_IF(lua_gettop(L)-top!=1, "Building the object pushed more than 1 value");
	int rc = luaL_getmetafield(L, -1, fnName.c_str());
	THROW_IF(rc==LUA_TNIL, "Unable to load the function " << fnName);
	THROW_IF(!lua_isfunction(L,-1), fnName << " is not a function");
	lua_insert(L,-2);
	int numArgs = 1 + args(L);
	rc = lua_pcall(L,numArgs, LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK,
		 "Error running " << fnName << ", rc " << rc
		 << ", err " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }
  void Util::LoadFile(const Exec::Ptr   &ePtr,
		      const Future::Ptr &future,
		      const std::string &path) {
    THROW_IF(!ePtr, "Call to LoadFile with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	int rc = luaL_loadfile(L, path.c_str());
	THROW_IF(rc!=LUA_OK, "Failed to load file"
		 << ", path="  << path
		 << ", error=" << lua_tostring(L,-1));
	rc = lua_pcall(L, 0, LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK, "failed to initialize " << path
		 << ", err = " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }
  void Util::LoadString(const Exec::Ptr   &ePtr,
			const Future::Ptr &future,
			const std::string &str) {
    THROW_IF(!ePtr, "Call to LoadString with uninitialized engine pointer");
    ePtr->Run(future, [=](lua_State *L) -> int {
	int rc = luaL_loadstring(L, str.c_str());
	THROW_IF(rc!=LUA_OK, "Failed to load string"
		 << "\n str=" << FirstLine(str)
		 << "\n error=" << lua_tostring(L,-1));
	rc = lua_pcall(L, 0, LUA_MULTRET, 0);
	THROW_IF(rc!=LUA_OK, "Failed to initialize " << FirstLine(str)
		 << ", err = " << lua_tostring(L,-1));
	return lua_gettop(L);
      });
  }

  void Util::LoadFnMap(const Exec::Ptr        &ePtr,
		       const Future::Ptr      &future,
		       const std::string      &path,
		       const FunctionMap::Ptr &fnMap) {
    THROW_IF(!ePtr, "Call to LoadFnMap with uninitialized engine pointer, path=" << path);
    THROW_IF(!fnMap, "Call to LoadFnMap with uninitialized function map, path="  << path);
    fnMap->Freeze();
    ePtr->Run(future, [=](lua_State *L) -> int {
	std::string baseName = path + ".";
	lua_pushglobaltable(L);
	int tblIdx = lua_absindex(L, -1);
	LoadTable(L, tblIdx, path);
	LoadFnMap(ePtr->Registry(), L, tblIdx, baseName, fnMap);
	return lua_gettop(L);
      });
  }

  // ****************************************************************************************
  // bare Lua calls
  int Util::GetElement(lua_State         *L,
		       const std::string &key) {
    lua_checkstack(L,2);
    static const char *chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.[]_";
    THROW_IF(key.find_first_not_of(chars)!=std::string::npos, "illegal key: " << key);
    std::string code = "return ";
    code.append(key);
    int rc = luaL_loadstring(L, code.c_str());
    THROW_IF(rc!=LUA_OK, "Failed to load code"
	     << ", rc = " << RCStr(rc)
	     << ", err = " << lua_tostring(L,-1));
    rc = lua_pcall(L, 0, 1, 0);
    THROW_IF(rc!=LUA_OK, "Failed to retrieve key"
	     << ", rc = " << RCStr(rc)
	     << ", err = " << lua_tostring(L,-1));
    return lua_type(L,-1);
  }
    
  void Util::LoadFnMap(const aliSystem::Registry::Ptr &rPtr,
		       lua_State                      *L,
		       int                             tblIdx,
		       const std::string              &baseName_,
		       const FunctionMap::Ptr         &fnMap) {
    StackGuard  sg(L,3);
    THROW_IF(!rPtr, "Registry pointer cannot be null");
    THROW_IF(lua_type(L,tblIdx)!=LUA_TTABLE, "Expecting a table reference");
    tblIdx = lua_absindex(L, tblIdx);
    fnMap->ForEach([=](const std::string &fnName, const LuaFn &fn) {
	std::string baseName = baseName_;
	baseName.append(fnName);
	FnObject::Ptr fnPtr(new FnObject(baseName, fn));
	rPtr->Register(fnPtr);
	lua_pushlightuserdata(L, fnPtr.get()); // UP_FN_OBJECT_PTR
	lua_pushcclosure(L, &HandleFn, 1);
	lua_setfield(L, tblIdx, fnName.c_str());
	return true;
      });
  }

    
  // ****************************************************************************************
  // bare calls
  int Util::GetRegistryKey() {
    static int        next=0;
    static std::mutex lock;
    std::lock_guard<std::mutex> g(lock);
    return ++next;
  }
  void Util::RegistrySet(lua_State *L, int key) {
    THROW_IF(lua_gettop(L)==0, "Nothing on stack to push");
    lua_rawseti(L, LUA_REGISTRYINDEX, key);
  }
  void Util::RegistryGet(lua_State *L, int key) {
    lua_checkstack(L,1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, key);
  }

}

