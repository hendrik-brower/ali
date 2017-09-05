#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using BC   = aliSystem::BasicCodec;
  using SObj = aliSystem::Codec::Serialize;
  using DObj = aliSystem::Codec::Deserialize;
}

TEST(aliSystemBasicCodec, general) {
  const std::string junk = "junk";
  const char       *name = "basicCodec";
  SObj::Ptr         sPtr = BC::GetSerializer();
  DObj::Ptr         dPtr = BC::GetDeserializer();
  ASSERT_STREQ(sPtr->Name().c_str(),name);
  ASSERT_STREQ(dPtr->Name().c_str(),name);
  ASSERT_EQ(sPtr->Version(),1u);
  ASSERT_EQ(dPtr->Version(),1u);
  ASSERT_TRUE( dPtr->CanDeserialize(name,0));
  ASSERT_TRUE( dPtr->CanDeserialize(name,1));
  ASSERT_FALSE(dPtr->CanDeserialize(name,2));
  ASSERT_FALSE(dPtr->CanDeserialize(junk,1));
}
//       enum class Type { END, INVALID, BOOL, INT, DOUBLE, STRING };

TEST(aliSystemBasicCodec, encDec) {
  SObj::Ptr         sPtr = BC::GetSerializer();
  DObj::Ptr         dPtr = BC::GetDeserializer();
  std::stringstream out;
  bool              b1 = true;
  int               i1 = -342;
  double            d1 = 2514630.234;
  std::string       s1 = "my string value\nline 2";
  sPtr->WriteBool  (out, b1);
  sPtr->WriteInt   (out, i1);
  sPtr->WriteDouble(out, d1);
  sPtr->WriteString(out, s1);
  ASSERT_EQ(dPtr->NextType  (out), DObj::Type::BOOL);
  ASSERT_EQ(dPtr->ReadBool  (out), b1);
  ASSERT_EQ(dPtr->NextType  (out), DObj::Type::INT);
  ASSERT_EQ(dPtr->ReadInt   (out), i1);
  ASSERT_EQ(dPtr->NextType  (out), DObj::Type::DOUBLE);
  ASSERT_EQ(dPtr->ReadDouble(out), d1);
  ASSERT_EQ(dPtr->NextType  (out), DObj::Type::STRING);
  std::string tmp;
  dPtr->ReadString(out, tmp);  
  ASSERT_STREQ(tmp.c_str(), s1.c_str());
  ASSERT_EQ(dPtr->NextType  (out), DObj::Type::END);
}

TEST(aliSystemBasicCodec, invalidData) {
  std::stringstream in;
  DObj::Ptr         dPtr = BC::GetDeserializer();
  in << "XXXX";
  ASSERT_EQ(dPtr->NextType(in), DObj::Type::INVALID);
}

