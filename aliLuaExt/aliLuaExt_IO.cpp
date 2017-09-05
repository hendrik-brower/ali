#include <aliLuaExt_IO.hpp>
#include <aliLuaExt_codec.hpp>
#include <aliLuaCore.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <cstring>
#include <iomanip>
#include <unordered_map>

namespace {
  using SOBJ = aliLuaExt::Codec::SOBJ;
  using DOBJ = aliLuaExt::Codec::DOBJ;
  
  int ToString(lua_State *L) {
    int               indentSize;
    std::string       separator;
    std::string       rootName;
    bool              enableNewLines;
    bool              showTableAddress;
    SOBJ::TPtr        serialize;
    THROW_IF(!lua_istable(L,1), "expecting a table of options");
    aliLuaCore::Table::GetInteger(L, 1, "indentSize",       indentSize,       true, 2);
    aliLuaCore::Table::GetString (L, 1, "separator",        separator,        true, ", ");
    aliLuaCore::Table::GetString (L, 1, "rootName",         rootName,         true, "root");
    aliLuaCore::Table::GetBool   (L, 1, "enableNewLines",   enableNewLines,   true, true);
    aliLuaCore::Table::GetBool   (L, 1, "showTableAddress", showTableAddress, true, false);
    SOBJ::GetTableValue(L, 1, "serialize", serialize, true);
    aliLuaExt::IOOptions opt(2, lua_gettop(L)-1);
    opt.SetIndentSize(indentSize);
    opt.SetSeparator(separator);
    opt.SetRootName(rootName);
    opt.SetEnableNewLines(enableNewLines);
    opt.SetShowTableAddress(showTableAddress);
    opt.SetSerialize(serialize);
    std::stringstream out;
    out << aliLuaExt::IO(L,opt);
    return aliLuaCore::Values::MakeString(L,out.str());
  }
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("IO functions");
    fnMap->Add("ToString",    ToString);
    fnMap->Add("Log",         aliLuaExt::IO::Log);
    aliLuaCore::Module::Register("load luaIO functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.io",  fnMap);
				 });
  }
  void Fini() {}

  struct IOCtrl {
    using TableMap = std::unordered_map<const void*, std::string>;
    IOCtrl(lua_State *L_, std::ostream &out_)
      : L(L_),
	isKey(false),
	enableNewLines(false),
	showTableAddress(false),
	onlyMapRecursion(false),
	indentSize(2),
	out(out_) {
    }
    lua_State         *L;
    bool               isKey;
    bool               enableNewLines;
    bool               showTableAddress;
    bool               onlyMapRecursion;
    size_t             indentSize;
    TableMap           tableMap;
    std::string        indent;
    std::string        prefix;
    std::string        tableName;
    std::ostream      &out;
    friend std::ostream &operator<<(std::ostream &out, const IOCtrl &o) {
      out << "IOCtrl:"
	  << "\nIskey            = " << YES_NO(o.isKey)
	  << "\nenableNewLines   = " << YES_NO(o.enableNewLines)
	  << "\nshowTableAddress = " << YES_NO(o.showTableAddress)
	  << "\nindentSize       = " << o.indentSize
	  << "\nindent           = " << o.indent     
	  << "\nprefix           = " << o.prefix
	  << "\ntableName        = " << o.tableName;
      return out;
    }
  };
  struct Guard {
    using ResetFn = std::function<void()>;
    Guard(const ResetFn &resetFn_) : resetFn(resetFn_) {}
    ~Guard() {
      resetFn();
    }
  private:
    ResetFn resetFn;
  };

  std::string GetKey(lua_State *L, int index) {
    const static std::string openBracket  = "[";
    const static std::string closeBracket = "]";
    const static std::string dot          = ".";
    char buf[25];
    int type = lua_type(L,index);
    if (type==LUA_TNUMBER) {
      if (lua_isinteger(L,index)) {
	long long val = lua_tointeger(L,index);
	snprintf(buf, sizeof(buf), "[%lli]", val);
      } else {
	snprintf(buf, sizeof(buf), "[%f]", lua_tonumber(L,index));
      }
      return buf;
    } else if (type==LUA_TSTRING) {
      return dot + aliLuaCore::Values::GetString(L,index);
    } else {
      aliLuaCore::StackGuard g(L,1);
      lua_pushvalue(L,index);
      snprintf(buf, sizeof(buf), "<%p>", lua_topointer(L,index));
      return buf;
    }
  }
  
  void Write(int index, IOCtrl *ctrl) {
    index = lua_absindex(ctrl->L,index);
    int type = lua_type(ctrl->L, index);
    if (ctrl->isKey && ctrl->enableNewLines) {
      ctrl->out << ctrl->prefix;
    }
    if (false) {
    } else if (type==LUA_TNIL) {
      ctrl->out << "nil";
    } else if (type==LUA_TBOOLEAN) {
      ctrl->out << (lua_toboolean(ctrl->L,index) ? "true" : "false");
    } else if (type==LUA_TNUMBER) {
      if (lua_isinteger(ctrl->L, index)) {
	ctrl->out << lua_tointeger(ctrl->L,index);
      } else {
	ctrl->out << lua_tonumber(ctrl->L,index);
      }
    } else if (type==LUA_TSTRING) {
      size_t len = 0;
      const char *str = lua_tolstring(ctrl->L,index, &len);
      //if (!isKey) { ctrl->out << "'"; }
      ctrl->out.write(str, len);
      //if (!isKey) { ctrl->out << "'"; }
    } else if (type==LUA_TTABLE) {
      lua_pushvalue(ctrl->L,index);
      const void *addr = lua_topointer(ctrl->L,-1);
      lua_pop(ctrl->L,1);
      IOCtrl::TableMap::const_iterator it = ctrl->tableMap.find(addr);
      if (it==ctrl->tableMap.end()) {
	ctrl->tableMap[addr] = ctrl->tableName;
	bool firstKey = true;
	bool isEmpty  = true;
	if (ctrl->showTableAddress) {
	  ctrl->out << addr << " ";
	}
	ctrl->out << "{";
	lua_checkstack(ctrl->L,2);
	lua_pushnil(ctrl->L);
	while (lua_next(ctrl->L,index)) {
	  isEmpty = false;
	  if (firstKey) {
	    if (ctrl->enableNewLines) {
	      ctrl->out << "\n";
	    }
	    firstKey = false;
	  }
	  {
	    Guard g([nSz   = ctrl->tableName.size(),
		     pSz   = ctrl->prefix.size(),
		     isKey = ctrl->isKey,
		     ctrl_ = ctrl] {
		      ctrl_->tableName.resize(nSz);
		      ctrl_->prefix.resize(pSz);
		      ctrl_->isKey = isKey;
		    });
	    if (lua_type(ctrl->L,-2)==LUA_TTABLE) {
	      ctrl->tableName.append(GetKey(ctrl->L,-2));
	    }
	    ctrl->prefix.append(ctrl->indent);
	    ctrl->isKey = true;
	    Write(-2, ctrl);
	  }
	  ctrl->out << " = ";
	  {
	    if (lua_type(ctrl->L,-1)==LUA_TTABLE) {
	      Guard g([nSz   = ctrl->tableName.size(),
		       pSz   = ctrl->prefix.size(),
		       ctrl_ = ctrl] {
			ctrl_->tableName.resize(nSz);
			ctrl_->prefix.resize(pSz);
		      });
	      ctrl->tableName.append(GetKey(ctrl->L,-2));
	      ctrl->prefix.append(ctrl->indent);
	      Write(-1, ctrl);
	    } else {
	      Write(-1, ctrl);
	    }
	  }
	  ctrl->out << ",";
	  if (ctrl->enableNewLines) {
	    ctrl->out << "\n";
	  } else {
	    ctrl->out << " ";
	  }
	  lua_pop(ctrl->L,1);
	}
	if (!isEmpty && ctrl->enableNewLines) {
	  ctrl->out << ctrl->prefix;
	}
	ctrl->out << "}";
	if (ctrl->onlyMapRecursion) {
	  ctrl->tableMap.erase(addr);
	}
      } else {
	ctrl->out << "<ref";
	if (ctrl->showTableAddress) {
	  ctrl->out << " " << it->first;
	}
	ctrl->out << "> " << it->second;
      }
    } else if (type==LUA_TUSERDATA || type==LUA_TLIGHTUSERDATA) {
      std::string str = aliLuaCore::Values::GetString(ctrl->L,index);
      if (str.empty()) {
	aliLuaCore::StackGuard g(ctrl->L,1);
	lua_pushvalue(ctrl->L, index);
	ctrl->out << "ud(" << lua_topointer(ctrl->L,-1) << ")";
      } else {
	ctrl->out << "ud(" << str << ")";
      }
    } else if (type==LUA_TTHREAD) {
      aliLuaCore::StackGuard g(ctrl->L,1);
      lua_pushvalue(ctrl->L, index);
      ctrl->out << "thread(" << lua_topointer(ctrl->L,-1) << ")";
    } else if (type==LUA_TNONE) {
      ctrl->out << "<<invalid>>";
    } else if (type==LUA_TFUNCTION) {
      aliLuaCore::StackGuard g(ctrl->L,1);
      lua_pushvalue(ctrl->L, index);
      ctrl->out << "fn(" << lua_topointer(ctrl->L,-1) << ")";
    } else {
      THROW("Unrecognized type " << type << " -> " << lua_typename(ctrl->L,type));
    }
  }
  
}

namespace aliLuaExt {

  void IO::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::IO", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  
  int IO::Log(lua_State *L) {
    lua_Debug dbg;
    IOOptions opt;
    opt.SetSeparator(" ");
    memset(&dbg, 0, sizeof(lua_Debug));
    int rc = lua_getstack(L,1,&dbg);
    if (rc!=1) {
      BASE("INFO (<lua>) " << IO(L,opt));
    } else {
      rc = lua_getinfo(L, "nSlL", &dbg);
      lua_pop(L,1);
      if (rc==0) {
	BASE("INFO (<lua>) " << IO(L,opt));
      } else {
	const char *src  = dbg.short_src ? dbg.short_src : "?";
	const char *name = dbg.name;
	const char *fwd  = strrchr(src, '/');
	if (fwd && *(fwd+1)) {
	  src = fwd+1;
	}
	if (name) {
	  BASE("INFO ("<< src << ":" << name << ":" << dbg.currentline  << ") " << IO(L,opt));
	} else {
	  BASE("INFO ("<< src << ":" << dbg.currentline  << ") " << IO(L,opt));
	}
      }
    }
    return 0;
  }

  IO::IO(lua_State *L_)
    : L(L_) {
  }
  IO::IO(lua_State *L_, int index)
    : L(L_),
      opt(IOOptions(index)) {
  }

  IO::IO(lua_State *L_, const IOOptions &opt_)
    : L(L_),
      opt(opt_) {
  }
  std::ostream &operator<<(std::ostream &out, const IO &o) {
    int    index          = o.opt.GetIndex();
    size_t count          = o.opt.GetCount();
    aliSystem::Codec::Serialize::Ptr sPtr = o.opt.GetSerialize();
    if (sPtr) {
      aliSystem::Codec::Serializer s(out, sPtr);
      aliLuaCore::Serialize::Write(o.L, index, count, s);
    } else {
      IOCtrl ctrl(o.L, out);
      ctrl.isKey            = false;
      ctrl.enableNewLines   = o.opt.GetEnableNewLines();
      ctrl.showTableAddress = o.opt.GetShowTableAddress();
      ctrl.indent           = std::string(o.opt.GetIndentSize(), ' ');
      ctrl.prefix           = "";
      for (size_t i=0; i<count; ++i) {
	if (count==1) {
	  ctrl.tableName = o.opt.GetRootName();
	} else {
	  std::stringstream ss;
	  ss << o.opt.GetRootName() << "[" << index+i << "]";
	  ctrl.tableName = ss.str();
	}
	if (lua_isnone(o.L, index+i)) {
	  break;
	} else {
	  if (i>0) {
	    out << o.opt.GetSeparator();
	  }
	  Write(index+i, &ctrl);
	}
      }
    }
    return out;
  }

  
}
