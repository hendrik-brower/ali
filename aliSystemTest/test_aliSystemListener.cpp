#include "gtest/gtest.h"
#include <aliSystem.hpp>

TEST(aliSystemListener, general) {
  using List = aliSystem::Listener<size_t>;
  std::string name = "some desc";
  size_t      cnt  = 0;
  size_t      sum  = 0;
  List::Ptr lPtr = List::Create(name, [&](size_t val) {
      ++cnt;
      sum+=val;
    });
  ASSERT_TRUE(lPtr);
  ASSERT_STREQ(name.c_str(), lPtr->Info().c_str());
  ASSERT_EQ(cnt,(size_t)0);
  lPtr->Notify(100);
  ASSERT_EQ(cnt,(size_t)1);
  lPtr->Notify(31);
  ASSERT_EQ(cnt,(size_t)2);
  ASSERT_EQ(sum,(size_t)131);
}
