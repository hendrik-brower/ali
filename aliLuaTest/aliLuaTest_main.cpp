#include "gtest/gtest.h"
#include <aliLuaCore.hpp>
#include <aliLuaExt.hpp>
#include <aliLuaExt2.hpp>
#include <aliLuaExt3.hpp>
#include <aliSystem.hpp>
#include <aliLuaTest_util.hpp>

void RegisterInitFini_ExternalObjectTest(aliSystem::ComponentRegistry &cr);
void RegisterInitFini_MTTest(aliSystem::ComponentRegistry &cr);
void RegisterInitFini_ObjectTargetTest(aliSystem::ComponentRegistry &cr);
void RegisterInitFini_ThreadingTest(aliSystem::ComponentRegistry &cr);

int main(int argc, char *argv[]) {
  aliSystem::ComponentRegistry cr;
  testing::InitGoogleTest(&argc, argv);
  aliSystem::RegisterInitFini(cr);
  aliLuaCore::RegisterInitFini(cr);
  aliLuaExt::RegisterInitFini(cr);
  aliLuaExt2::RegisterInitFini(cr);
  aliLuaExt3::RegisterInitFini(cr);
  //
  // register test init/fini's
  RegisterInitFini_ExternalObjectTest(cr);
  RegisterInitFini_MTTest(cr);
  RegisterInitFini_ObjectTargetTest(cr);
  RegisterInitFini_ThreadingTest(cr);
  aliLuaTest::Util::RegisterInitFini(cr);

  //
  // start up the testing
  cr.Init();
  int rc = RUN_ALL_TESTS();
  return rc;
}

