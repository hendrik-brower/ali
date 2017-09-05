#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using Deserialize  = aliSystem::Codec::Deserialize;
  using Deserializer = aliSystem::Codec::Deserializer;
  using Serialize    = aliSystem::Codec::Serialize;
  using Serializer   = aliSystem::Codec::Serializer;

  struct S : public Serialize {
    S() : Serialize("dummy", 3u) {}
    void WriteBool  (std::ostream &out, bool val)               { out << val << std::endl; }
    void WriteInt   (std::ostream &out, int val)                { out << val << std::endl; }
    void WriteDouble(std::ostream &out, double val)             { out << val << std::endl; }
    void WriteString(std::ostream &out, const std::string &val) { out << val << std::endl; }
    void WriteString(std::ostream &out, const char *data, size_t len) {
      std::string s(data,len);
      WriteString(out,s);
    }
  };
  
  struct D : public Deserialize {
    D() : Deserialize("dummy", 4u) {}
    Type   NextType  (std::istream &)   { return Type::END; }
    bool   ReadBool  (std::istream &in) { bool   v; in >> v; return v; }
    int    ReadInt   (std::istream &in) { int    v; in >> v; return v; }
    double ReadDouble(std::istream &in) { double v; in >> v; return v; }
    std::string &ReadString(std::istream &in, std::string &data) {
      in >> data;
      return data;
    }
  };
  
}

TEST(aliSystemCodec, versionChecks) {
  S s;
  D d;
  ASSERT_STREQ(s.Name().c_str(), "dummy");
  ASSERT_STREQ(d.Name().c_str(), "dummy");
  ASSERT_EQ(s.Version(), 3u);
  ASSERT_EQ(d.Version(), 4u);
  ASSERT_TRUE( d.CanDeserialize("dummy", 0));
  ASSERT_TRUE( d.CanDeserialize("dummy", 3));
  ASSERT_TRUE( d.CanDeserialize("dummy", 4));
  ASSERT_FALSE(d.CanDeserialize("dummy", 5));
  ASSERT_FALSE(d.CanDeserialize("DUMMY", 5));
}

TEST(aliSystemCodec, general) {
  S s;
  D d;
  bool   b1 = true,   b2 = false;
  int    i1 = 4,      i2 = 3242;
  double d1 = 2531.42,d2 = 242.66;
  std::string s1="THIS", s2="THAT", s3="HERE", s4="THERE";
  std::string tmp;
  std::stringstream ss;
  s.WriteBool(ss,b1);
  s.WriteBool(ss,b2);
  s.WriteInt(ss, i1);
  s.WriteInt(ss, i2);
  s.WriteDouble(ss, d1);
  s.WriteDouble(ss, d2);
  s.WriteString(ss, s1);
  s.WriteString(ss, s2);
  s.WriteString(ss, s3.c_str(), s3.size());
  s.WriteString(ss, s4.c_str(), s4.size());
  ASSERT_EQ(d.ReadBool  (ss),b1);
  ASSERT_EQ(d.ReadBool  (ss),b2);
  ASSERT_EQ(d.ReadInt   (ss),i1);
  ASSERT_EQ(d.ReadInt   (ss),i2);
  ASSERT_EQ(d.ReadDouble(ss),d1);
  ASSERT_EQ(d.ReadDouble(ss),d2);
  ASSERT_STREQ(d.ReadString(ss,tmp).c_str(),s1.c_str());
  ASSERT_STREQ(d.ReadString(ss,tmp).c_str(),s2.c_str());
  ASSERT_STREQ(d.ReadString(ss,tmp).c_str(),s3.c_str());
  ASSERT_STREQ(d.ReadString(ss,tmp).c_str(),s4.c_str());
}

TEST(aliSystemCodec, serializer) {
  Serialize::Ptr   sPtr(new S);
  Deserialize::Ptr dPtr(new D);
  std::stringstream ss;
  bool   b1 = true,   b2 = false;
  int    i1 = 4,      i2 = 3242;
  double d1 = 2531.42,d2 = 242.66;
  std::string s1="THIS", s2="THAT", s3="HERE", s4="THERE";
  std::string tmp;
  Serializer   s(ss, sPtr);
  Deserializer d(ss, dPtr);
  ASSERT_STREQ(s.Name().c_str(), "dummy");
  ASSERT_STREQ(d.Name().c_str(), "dummy");
  ASSERT_EQ(s.Version(), 3u);
  ASSERT_EQ(d.Version(), 4u);
  ASSERT_TRUE( d.CanDeserialize("dummy", 0));
  ASSERT_TRUE( d.CanDeserialize("dummy", 3));
  ASSERT_TRUE( d.CanDeserialize("dummy", 4));
  ASSERT_FALSE(d.CanDeserialize("dummy", 5));
  ASSERT_FALSE(d.CanDeserialize("DUMMY", 5));
  s.WriteBool(b1);
  s.WriteBool(b2);
  s.WriteInt(i1);
  s.WriteInt(i2);
  s.WriteDouble(d1);
  s.WriteDouble(d2);
  s.WriteString(s1);
  s.WriteString(s2);
  s.WriteString(s3.c_str(), s3.size());
  s.WriteString(s4.c_str(), s4.size());
  ASSERT_EQ(d.ReadBool  (),b1);
  ASSERT_EQ(d.ReadBool  (),b2);
  ASSERT_EQ(d.ReadInt   (),i1);
  ASSERT_EQ(d.ReadInt   (),i2);
  ASSERT_EQ(d.ReadDouble(),d1);
  ASSERT_EQ(d.ReadDouble(),d2);
  ASSERT_STREQ(d.ReadString(tmp).c_str(),s1.c_str());
  ASSERT_STREQ(d.ReadString(tmp).c_str(),s2.c_str());
  ASSERT_STREQ(d.ReadString(tmp).c_str(),s3.c_str());
  ASSERT_STREQ(d.ReadString(tmp).c_str(),s4.c_str());
}
