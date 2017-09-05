#include <aliSystem_basicCodec.hpp>
#include <aliSystem_logging.hpp>
#include <algorithm>
#include <arpa/inet.h>

namespace {

  //
  // NOTES:
  //    int/double are stored in little endian byte order
  //    bool is stored as t/f
  //    int  is stored as i<<4bytes>>
  //    double is stored as d<<8bytes>>
  //    string is stored as one of:
  //       a)  s
  //       b)  S<<4byte len>><<len bytes>>
  //       c)  <<1 byte (len+128)>><<X-128 bytes>>
  //       where:
  //       a) is a null/zero byte length string
  //       b) is a string >=128 bytes long
  //       c) is a string >0 and <128 bytes long
  //
  
  using CPtr = std::unique_ptr<char[]>;
  
  const std::string codecName    = "basicCodec";
  const size_t      codecVersion = 1;
  aliSystem::Codec::Serialize  ::Ptr basicSerializer;
  aliSystem::Codec::Deserialize::Ptr basicDeserializer;
    
  struct BS : public aliSystem::Codec::Serialize {
    BS();
    void Write(std::ostream &out, const char *cp, size_t len);
    void WriteNumber(std::ostream &out, char     *val, size_t sz);
    void WriteBool  (std::ostream &out, bool      val) override;
    void WriteInt   (std::ostream &out, int       val) override;
    void WriteDouble(std::ostream &out, double    val) override;
    void WriteString(std::ostream &out, const std::string &data) override;
    void WriteString(std::ostream &out, const char *data, size_t len) override;
  };
  struct BD : public aliSystem::Codec::Deserialize {
    BD();
    int  Peek(std::istream &in);
    void Verify(std::istream &in, char exp, const std::string type);
    void Read(std::istream &in, char *v, size_t sz);
    void ReadNumber(std::istream &in, char *v, size_t sz);
    Type         NextType(std::istream &in);
    bool         ReadBool(std::istream &in);
    int          ReadInt(std::istream &in);
    double       ReadDouble(std::istream &in);
    std::string &ReadString(std::istream &in, std::string &data);
  };

  // ****************************************************************************************
  // Serializer Implementation
  BS::BS()
    : aliSystem::Codec::Serialize(codecName, codecVersion) {
  }
  void BS::Write(std::ostream &out, const char *cp, size_t len) {
    std::string str(cp,len);
    out.write(cp,len);
  }
  
  void BS::WriteNumber(std::ostream &out, char *val, size_t sz) {
    if (htonl(1)==1) {
      // big endian machine, convert value to little endian
      std::reverse((char*)&val, ((char*)&val)+sizeof(val));
    }
    out.write(val,sz);
  }

  void BS::WriteBool(std::ostream &out, bool val) {
    Write(out,"b",1);
    THROW_IF(out.bad(), "Failed to write boolean type");
    Write(out, val ? "t" : "f", 1);
    THROW_IF(out.bad(), "Failed to write boolean");
  }
  void BS::WriteInt(std::ostream &out, int val) {
    Write(out,"i",1);
    THROW_IF(out.bad(), "Failed to write int type");
    WriteNumber(out, (char*)&val, sizeof(int));
    THROW_IF(out.bad(), "Failed to write int");
  }
  void BS::WriteDouble(std::ostream &out, double val) {
    Write(out,"d",1);
    THROW_IF(out.bad(), "Failed to write double type");
    WriteNumber(out,(char*)&val, sizeof(double));
    THROW_IF(out.bad(), "Failed to write double");
  }
  void BS::WriteString(std::ostream &out, const std::string &data) {
    WriteString(out, data.c_str(), data.size());
  }
  void BS::WriteString(std::ostream &out, const char *str, size_t len) {
    if (len==0 || str==nullptr) {
      // nullptr  or zero length string are converted to zero length strings
      Write(out,"s",1);
      THROW_IF(out.bad(), "Failed to write string type");
    } else if (len<128) {
      //
      // short strings [1-128) characters long
      unsigned char l = (unsigned char)len + 128;
      WriteNumber(out,(char*)&l, sizeof(unsigned char));
      THROW_IF(out.bad(), "Failed to write string type");
      Write(out, str, len);
      THROW_IF(out.bad(), "Failed to write string");
    } else {
      //
      // long strings [128,MAX_INT] characters long
      THROW_IF(len>std::numeric_limits<unsigned int>::max(), "string exceeds size limits");
      Write(out,"S",1);
      THROW_IF(out.bad(), "Failed to write string type");
      unsigned int l = len;  // copy since WriteNumber might alter the value
      WriteNumber(out, (char*)&l, sizeof(unsigned int));
      THROW_IF(out.bad(), "Failed to write string length");
      Write(out, str, len);
      THROW_IF(out.bad(), "Failed to write string");
    }
  }


  // ****************************************************************************************
  // Deserializer Implementation
  BD::BD()
    : aliSystem::Codec::Deserialize(codecName, codecVersion) {
  }
  int BD::Peek(std::istream &in) {
    int c = in.peek();
    std::string str((char*)&c,sizeof(int));
    return c;
  }
  void BD::Verify(std::istream &in, char exp, const std::string type) {
    int t = in.peek();
    std::string str((char*)&t,sizeof(int));
    THROW_IF(t!=exp, "next element is not a " << type << " t=" << t);
    char v;
    in.read(&v,1);
    str = std::string(&v,1);
    THROW_IF(in.bad(), "failed to read the " << type << " type");
    THROW_IF(v!=exp, "invalid " << type << " prefix v=" << (int)v);
  }
  void BD::Read(std::istream &in, char *v, size_t sz) {
    in.read(v,sz);
    std::string str(v,sz);
  }
  void BD::ReadNumber(std::istream &in, char *v, size_t sz) {
    Read(in, v, sz);
    if (htonl(1)==1) {
      // big endian machine, convert value to big endian
      std::reverse(v, v+sz);
    }
  }
  BD::Type BD::NextType(std::istream &in) {
    int c = Peek(in);
    if (false) {
    } else if (c==EOF  ) { return Type::END;
    } else if (in.bad()) { return Type::INVALID;
    } else if (c=='b'  ) { return Type::BOOL;
    } else if (c=='i'  ) { return Type::INT;
    } else if (c=='d'  ) { return Type::DOUBLE;
    } else if (c=='s'  ) { return Type::STRING;
    } else if (c=='S'  ) { return Type::STRING;
    } else if (c>128   ) { return Type::STRING;
    }
    return Type::INVALID;
  }
  bool BD::ReadBool(std::istream &in) {
    Verify(in, 'b', "boolean");
    char v;
    Read(in,&v,1);
    THROW_IF(in.bad(), "failed to read the boolean value");
    if (v=='t') {
      return true;
    } else if (v=='f') {
      return false;
    } else {
      THROW("invalid boolean character v=" << (int)v);
    }
  }
  int BD::ReadInt(std::istream &in) {
    Verify(in, 'i', "integer");
    int val;
    ReadNumber(in,(char*)&val, sizeof(int));
    THROW_IF(in.bad(), "invalid int");
    return val;
  }
  double BD::ReadDouble(std::istream &in) {
    Verify(in, 'd', "double");
    double val;
    ReadNumber(in,(char*)&val, sizeof(double));
    THROW_IF(in.bad(), "invalid double");
    return val;
  }
  std::string &BD::ReadString(std::istream &in, std::string &data) {
    int p = Peek(in);
    if (p=='s') {
      // empty string
      char t;
      Read(in,&t,1);
      THROW_IF(in.bad(), "Failed to read string type");
      THROW_IF(t!='s', "Unexpected string type t=" << (int)t);
      data.clear();
    } else if (p=='S') {
      // long string
      char t;
      Read(in,&t,1);
      THROW_IF(in.bad(), "Failed to read string type");
      THROW_IF(t!='S', "Unexpected string type t=" << (int)t);
      unsigned int len;
      ReadNumber(in,(char*)&len, sizeof(unsigned int));
      THROW_IF(in.bad(), "Failed to read string length");
      CPtr buf(new char[len]);
      Read(in,buf.get(), len);
      THROW_IF(in.bad(), "failed to read string");
      data = std::string(buf.get(), len);
    } else if (p>128 && p<256) {
      // short string
      char buf[128];
      unsigned char t;
      ReadNumber(in,(char*)&t,1);
      THROW_IF(in.bad(), "Failed to read string type");
      THROW_IF(t<=128, "Unexpected string type t=" << (int)t);
      size_t len = t - 128;
      THROW_IF(len>=128, "invalid string length " << len);
      Read(in, buf, len);
      THROW_IF(in.bad(), "truncated input"); // FIXME: || in.eof()?
      data = std::string(buf, len);
    } else {
      THROW("next element is not a string p=" << p);
    }
    return data;
  }
    
  void Init() {
    // VERIFY IEEE-754?
    THROW_IF(sizeof(unsigned char)!=1, "Unsupported unsigned char size");
    THROW_IF(sizeof(int)!=4,           "Unsupported int size");
    THROW_IF(sizeof(double)!=8,        "Unsupported double size");
    basicSerializer  .reset(new BS);
    basicDeserializer.reset(new BD);
  }
    
  void Fini() {
    basicSerializer  .reset();
    basicDeserializer.reset();
  }
    
}
  
namespace aliSystem {
  
  void BasicCodec::RegisterInitFini(aliSystem::ComponentRegistry &cr) {
    aliSystem::Component::Ptr ptr = cr.Register("aliSystem::BasicCodec", Init, Fini);
  }
  
  Codec::Serialize::Ptr BasicCodec::GetSerializer() {
    Codec::Serialize::Ptr rtn = basicSerializer;
    THROW_IF(!rtn, "Basic serializer is not defined");
    return rtn;
  }
  
  Codec::Deserialize::Ptr BasicCodec::GetDeserializer() {
    Codec::Deserialize::Ptr rtn = basicDeserializer;
    THROW_IF(!rtn, "Basic deserializer is not defined");
    return rtn;
  }
  
}
