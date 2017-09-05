#include "gtest/gtest.h"
#include <aliSystem.hpp>


int main(int argc, char *argv[]) {
  aliSystem::ComponentRegistry cr;
  testing::InitGoogleTest(&argc, argv);
  aliSystem::RegisterInitFini(cr);
  cr.Init();
  int rc = RUN_ALL_TESTS();
  return rc;
}
