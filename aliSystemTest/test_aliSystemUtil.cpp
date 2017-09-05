#include "gtest/gtest.h"
#include <aliSystem.hpp>


TEST(aliSystemUtil, DoNothing) {
  aliSystem::Util::DoNothing();
  EXPECT_EQ(true,true);
}
TEST(aliSystemUtil, MatchAll) {
  aliSystem::Util::StrSelectFn fn = aliSystem::Util::MatchAll<std::string>;
  EXPECT_TRUE(fn("Junk1"));
  EXPECT_TRUE(fn("Junk2"));
  EXPECT_TRUE(fn(""));
}
TEST(aliSystemUtil, GetMatchShortestFn) {
  aliSystem::Util::StrSelectFn fn = aliSystem::Util::GetMatchShortestFn("Jun");
  EXPECT_TRUE(fn("Junk1"));
  EXPECT_TRUE(fn("Ju"));
  EXPECT_TRUE(fn(""));
}
TEST(aliSystemUtil, GetMatchAllFn) {
  aliSystem::Util::StrSelectFn fn = aliSystem::Util::MatchAll<std::string>;
  EXPECT_TRUE(fn("Junk1"));
  EXPECT_TRUE(fn("Junk2"));
  EXPECT_TRUE(fn(""));
}
TEST(aliSystemUtil, GetMatchExactFn) {
  aliSystem::Util::StrSelectFn fn = aliSystem::Util::GetMatchExactFn("value1");
  EXPECT_TRUE( fn("value1"));
  EXPECT_FALSE(fn("value2"));
  EXPECT_FALSE(fn(""));
  EXPECT_TRUE( fn("value1"));
  EXPECT_FALSE(fn("value2"));
  EXPECT_FALSE(fn(""));
}
TEST(aliSystemUtil, GetMatchPrefixFn) {
  aliSystem::Util::StrSelectFn fn = aliSystem::Util::GetMatchPrefixFn("prefix");
  EXPECT_TRUE( fn("prefixX"));
  EXPECT_FALSE(fn("prefiXX"));
  EXPECT_FALSE(fn(""));
  EXPECT_FALSE(fn("pre"));
  EXPECT_TRUE( fn("prefix"));
  EXPECT_FALSE(fn("xyz"));
  EXPECT_FALSE(fn("PREFIX"));
}
TEST(aliSystemUtil, Always) {    
  EXPECT_TRUE(aliSystem::Util::Always());
}
TEST(aliSystemUtil, Never) {    
  EXPECT_FALSE(aliSystem::Util::Never());
}


