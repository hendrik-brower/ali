#include <aliLuaExt_hold.hpp>

namespace {

  using OBJ = aliLuaExt::Hold::OBJ;

  int Create(lua_State *L) {
    std::string file, reason;
    int         line;
    aliLuaCore::Table::GetString (L, 1, "file",   file,   true, __FILE__);
    aliLuaCore::Table::GetString (L, 1, "reason", reason, true, "unspecified");
    aliLuaCore::Table::GetInteger(L, 1, "line",   line,   true);
    OBJ::TPtr   hPtr   = aliSystem::Hold::Create(file, line, reason);
    return OBJ::Make(L,hPtr);
  }
  int GetHolds(lua_State *L) {
    aliSystem::Hold::Vec vec;
    aliSystem::Hold::GetHolds(vec);
    aliLuaCore::MakeTableUtil tbl;
    for (aliSystem::Hold::Vec::iterator it=vec.begin();it!=vec.end();++it) {
      OBJ::TPtr hPtr = *it;
      tbl.SetMakeFnForIndex(1+it-vec.begin(), OBJ::GetMakeFn(hPtr));
    }
    return tbl.Make(L);
  }
  int GetInfo(lua_State *L) {
    OBJ::TPtr hPtr = OBJ::Get(L,1,false);
    aliLuaCore::MakeTableUtil tbl;
    tbl.SetString("file",   std::string(hPtr->GetFile()));
    tbl.SetNumber("line",   (int)hPtr->GetLine());
    tbl.SetString("reason", hPtr->GetReason());
    return tbl.Make(L);
  }
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("hold functions");
    fnMap->Add("Create",   Create);
    fnMap->Add("GetHolds", GetHolds);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("hold MT");
    mtMap->Add("GetInfo", GetInfo);
    OBJ::Init("luaHold", mtMap, true);
    aliLuaCore::Module::Register("load aliLuaExt::Hold functions",
			     [=](const aliLuaCore::Exec::Ptr &ePtr) {
			       aliLuaCore::Util::LoadFnMap(ePtr, "lib.aliLua.hold",  fnMap);
			       OBJ::Register(ePtr);
			     });
  }
  void Fini() {
    OBJ::Fini();
  }
  
}

namespace aliLuaExt {

  void Hold::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt::Hold", Init, Fini);
    ptr->AddDependency("aliSystem");
    ptr->AddDependency("aliLuaCore");
  }
  
}
