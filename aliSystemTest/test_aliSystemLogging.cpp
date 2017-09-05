#include "gtest/gtest.h"
#include <aliSystem.hpp>

TEST(aliSystemLogging, general) {
  int caught = 0;
  DEBUG("debug statement");
  INFO("debug statement");
  WARN("debug statement");
  ERROR("debug statement");
  ASSERT_EQ(caught,0);
  try {
    THROW("throwing an exception");
  } catch (std::exception &e) {
    ++caught;
  }
  ASSERT_EQ(caught,1);
  try {
    THROW_IF(false,"throwing an exception");
  } catch (std::exception &e) {
    ++caught;
  }
  ASSERT_EQ(caught,1);
  try {
    THROW_IF(true,"throwing an exception");
  } catch (std::exception &e) {
    ++caught;
  }
  ASSERT_EQ(caught,2);
}
