#include "gtest/gtest.h"
#include <aliLuaExt.hpp>

namespace {
  using Opt = aliLuaExt::IOOptions;
}

TEST(aliLuaExtIOOptions, defaults) {
  Opt opt;
  ASSERT_EQ(opt.GetIndex(), 1);
  ASSERT_EQ(opt.GetCount(), std::numeric_limits<size_t>::max());
  ASSERT_EQ(opt.GetIndentSize(), 2u);
  ASSERT_STREQ(opt.GetSeparator().c_str(), ", ");
  ASSERT_STREQ(opt.GetRootName().c_str(), "root");
  ASSERT_TRUE(opt.GetEnableNewLines());
  ASSERT_FALSE(opt.GetShowTableAddress());
  ASSERT_FALSE(opt.GetSerialize());
}

TEST(aliLuaExtIOOptions, constructorIndex) {
  const int index = 5;
  Opt opt(index);
  ASSERT_EQ(opt.GetIndex(), index);
  ASSERT_EQ(opt.GetCount(), std::numeric_limits<size_t>::max());
}
TEST(aliLuaExtIOOptions, constructorCount) {
  const int    index = 3;
  const size_t count = 9;
  Opt opt(index, count);
  ASSERT_EQ(opt.GetIndex(), index);
  ASSERT_EQ(opt.GetCount(), count);
}
TEST(aliLuaExtIOOptions, setGetIndex) {
  Opt opt;
  opt.SetIndex(3);
  ASSERT_EQ(opt.GetIndex(), 3);
}

TEST(aliLuaExtIOOptions, setGetCount) {
  Opt opt;
  opt.SetCount(5);
  ASSERT_EQ(opt.GetCount(), 5u);
}

TEST(aliLuaExtIOOptions, setGetIndent) {
  Opt opt;
  opt.SetIndentSize(11);
  ASSERT_EQ(opt.GetIndentSize(), 11u);
}

TEST(aliLuaExtIOOptions, setGetSeparator) {
  const char *sep = "...|...";
  Opt opt;
  opt.SetSeparator(sep);
  ASSERT_STREQ(opt.GetSeparator().c_str(), sep);
}

TEST(aliLuaExtIOOptions, setGetRootName) {
  const char *root = "<<<<ROOT>>>>";
  Opt opt;
  opt.SetRootName(root);
  ASSERT_STREQ(opt.GetRootName().c_str(), root);
}

TEST(aliLuaExtIOOptions, setGetEnableNewLines) {
  Opt opt;
  opt.SetEnableNewLines(false);
  ASSERT_FALSE(opt.GetEnableNewLines());
}
TEST(aliLuaExtIOOptions, setGetShowTableAddress) {
  Opt opt;
  opt.SetShowTableAddress(true);
  ASSERT_TRUE(opt.GetShowTableAddress());
}
TEST(aliLuaExtIOOptions, setGetSerialize) {
  Opt opt;
  opt.SetSerialize(aliSystem::BasicCodec::GetSerializer());
  ASSERT_TRUE(opt.GetSerialize());
}

