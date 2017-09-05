#include <aliLuaExt3_externalMT.hpp>
#include <aliLuaExt3_externalObject.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <lua.hpp>
#include <future>

namespace {
  using MTMap = std::map<std::string, aliLuaExt3::ExternalMT::WPtr>;
  using OBJ   = aliLuaExt3::ExternalMT::OBJ;

  std::mutex mtMapLock;
  MTMap      mtMap;

  void Register(aliLuaExt3::ExternalMT::Ptr emtPtr) {
    std::lock_guard<std::mutex> g(mtMapLock);
    MTMap::iterator it=mtMap.find(emtPtr->MTName());
    if (it!=mtMap.end()) {
      THROW_IF(!it->second.expired(), "External MT already defined for " << emtPtr->MTName());
    } 
    mtMap[emtPtr->MTName()] = emtPtr;
  }
    
  int Create(lua_State *L) {
    std::string                 mtName;
    std::string                 script;
    std::string                 createFn;
    std::string                 onDestroyFn;
    aliLuaCore::Table::SSet     functions;
    aliLuaExt3::ExternalMT::Ptr emtPtr;
    THROW_IF(!lua_istable(L, 1), "Expected a table argument");
    aliLuaCore::Table::GetString(L, 1, "mtName",        mtName,        false);
    aliLuaCore::Table::GetString(L, 1, "script",        script,        false);
    aliLuaCore::Table::GetString(L, 1, "createFn",      createFn,      false);
    aliLuaCore::Table::GetString(L, 1, "onDestroyFn",   onDestroyFn,   false);
    aliLuaCore::Table::GetStringSequence(L, 1, "functions", functions);
    emtPtr = aliLuaExt3::ExternalMT::Create(mtName,
					    script,
					    createFn,
					    onDestroyFn,
					    functions);
    return OBJ::Make(L, emtPtr);
  }
  int GetInfo(lua_State *L) {
    OBJ::TPtr emtPtr = OBJ::Get(L,1,false);
    aliLuaCore::MakeFn mkInfo = emtPtr->GetInfo();
    return mkInfo(L);
  }
    
    
  void Init() {
    aliLuaCore::FunctionMap::Ptr fnMap = aliLuaCore::FunctionMap::Create("external MT functions");
    fnMap->Add("Create", Create);
    aliLuaCore::FunctionMap::Ptr mtMap = aliLuaCore::FunctionMap::Create("external MT mt");
    mtMap->Add("GetInfo", GetInfo);
    OBJ::Init("aliExternalMT",  mtMap,  true);
    aliLuaCore::Module::Register("load aliLuaExt3::ExternalMT functions",
				 [=](const aliLuaCore::Exec::Ptr &ePtr) {
				   aliLuaCore::Util::LoadFnMap(ePtr,
							       "lib.aliLua.externalMT",
							       fnMap);
				   OBJ::Register(ePtr);
				 });
  }
  void Fini() {
    std::lock_guard<std::mutex> g(mtMapLock);
    mtMap.clear();
    OBJ::Fini();
  }
    
}

namespace aliLuaExt3 {


  void ExternalMT::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliLuaExt3::ExternalMT", Init, Fini);
    ptr->AddDependency("aliLuaCore");
  }
  ExternalMT::Ptr ExternalMT::Get(const std::string &mtName) {
    Ptr rtn;
    std::lock_guard<std::mutex> g(mtMapLock);
    MTMap::iterator it = mtMap.find(mtName);
    if (it!=mtMap.end()) {
      rtn = it->second.lock();
      if (!rtn) {
	mtMap.erase(it);
      }
    }
    return rtn;
  }
  ExternalMT::Ptr ExternalMT::Create(const std::string &mtName,
				     const std::string &script,
				     const std::string &createFn,
				     const std::string &onDestroyFn,
				     const SSet &functions) {
    ExternalMT::Ptr  emtPtr(new ExternalMT);
    emtPtr->mtName        = mtName;
    emtPtr->script        = script;
    emtPtr->createFn      = createFn;
    emtPtr->onDestroyFn   = onDestroyFn;
    emtPtr->functions     = functions;
    THROW_IF(emtPtr->functions.size()==0, "No functions defined for the object");
    Register(emtPtr);
    return emtPtr;
  }
  ExternalMT::~ExternalMT() {}  
  const std::string      &ExternalMT::MTName       () const { return mtName;        }
  const std::string      &ExternalMT::Script       () const { return script;        }
  const std::string      &ExternalMT::CreateFn     () const { return createFn;      }
  const std::string      &ExternalMT::OnDestroyFn  () const { return onDestroyFn;   }
  const ExternalMT::SSet &ExternalMT::Functions    () const { return functions;     }
  bool ExternalMT::IsDefined(const std::string &fnName) const {
    return functions.find(fnName)!=functions.end();
  }
  aliLuaCore::MakeFn ExternalMT::GetInfo() {
    aliLuaCore::MakeTableUtil mtu;
    aliLuaCore::MakeTableUtil::Ptr fnList = mtu.CreateSubtable("functions");
    mtu.SetString("mtName",        mtName);
    mtu.SetString("script",        script);
    mtu.SetString("createFn",      createFn);
    mtu.SetString("onDestroyFn",   onDestroyFn);
    size_t index=0;
    for (SSet::const_iterator it=functions.begin();
	 it!=functions.end();
	 ++it) {
      fnList->SetStringForIndex(++index, *it);
    }
    return mtu.GetMakeFn();
  }


  ExternalMT::ExternalMT() {}

}
