#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliSystem.hpp>
#include <app1.hpp>
#include <app2.hpp>
#include <app3.hpp>
#include <app4.hpp>
#include <exception>

const int RC_OK                = 0;
const int RC_ERROR_THROWN      = 1;
const int RC_ERROR_RESULT      = 2;
const int RC_ERROR_NO_FILE     = 3;

using ExecEngine = aliLuaExt::ExecEngine;
using Future     = aliLuaCore::Future;
using SVec       = aliLuaCore::Util::SVec;
using Hold       = aliSystem::Hold;
using Pool       = aliSystem::Threading::Pool;
using FL         = aliSystem::Listeners<Future*>;

namespace {
  Hold::Ptr hold;
  int rc = RC_OK;
}

int main(int argc, char *argv[]) {
  if (argc<2) {
    rc = RC_ERROR_NO_FILE;
    INFO("No file");
  } else {
    try {
      SVec args;
      for (int i=3; i<argc; ++i) {
	args.push_back(argv[i]);
      }
      aliSystem::ComponentRegistry cr;
      aliSystem::RegisterInitFini(cr);
      aliLuaCore::RegisterInitFini(cr);
      aliLuaExt::RegisterInitFini(cr);
      app1::RegisterInitFini(cr);
      app2::RegisterInitFini(cr);
      app3::RegisterInitFini(cr);
      app4::RegisterInitFini(cr);
      cr.Init();

      std::string     engineName = "engineName";
      std::string     poolName   = "poolName";
      std::string     fileName   = argv[1];
      Pool::Ptr       pool       = Pool::Create(poolName, 1);
      ExecEngine::Ptr engine     = ExecEngine::Create(engineName, pool);
      Future::Ptr     result     = Future::Create();
      hold = Hold::Create(__FILE__, __LINE__, "main script");
      FL::Register(result->OnSet(), "Done", [=](Future*future) -> void {
	  if (future->IsError()) {
	    rc = RC_ERROR_RESULT;
	  }
	  hold.reset();
	}, false);
      aliLuaCore::Util::Run(engine, [=](lua_State *L) {
	  lua_newtable(L);
	  for (int i=2; i<argc; ++i) {
	    lua_pushstring(L,argv[i]);
	    lua_rawseti(L,-2,i-1);
	  }
	  lua_setglobal(L, "args");
	  return 0;
	});
      aliLuaCore::Util::LoadFile(engine, result, fileName);
      aliSystem::Hold::WaitForHolds();
    } catch (std::exception &e) {
      rc = RC_ERROR_THROWN;
      ERROR("Exiting with an error " << e.what());
    }
  }
  return rc;
}
