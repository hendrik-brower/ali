#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  struct RegObj {
    using Ptr  = std::shared_ptr<RegObj>;
    using WPtr = std::weak_ptr<RegObj>;
    RegObj(size_t *val_)
      : val(val_) {
    }
    size_t Val() const { return *val; }
  private:
    size_t *val;
  };
}

TEST(aliSystemRegistry, general) {
  const std::string name = "myRegistry";
  size_t val = 0;
  aliSystem::Registry::Ptr reg = aliSystem::Registry::Create(name);
  ASSERT_TRUE(reg);
  ASSERT_STREQ(reg->Name().c_str(), name.c_str());
  RegObj::Ptr                   ptr(new RegObj(&val));
  RegObj::WPtr                  wPtr(ptr);
  const void                   *addr = ptr.get();
  aliSystem::Registry::ItemPtr  vPtr;
  reg->Register(ptr);
  ASSERT_EQ(ptr->Val(), (size_t)0);
  ++val;
  ASSERT_EQ(ptr->Val(), (size_t)1);
  ptr.reset();
  ++val;
  ASSERT_FALSE(ptr);
  vPtr = reg->Get(addr);
  ASSERT_TRUE(vPtr);
  ptr = std::static_pointer_cast<RegObj>(vPtr);
  ASSERT_EQ(ptr->Val(), val);
  reg->Unregister(addr);
  vPtr = reg->Get(addr);
  ASSERT_FALSE(vPtr);
  reg->Register(ptr);
  vPtr = reg->Get(addr);
  ASSERT_TRUE(vPtr);
  reg->Unregister(ptr);
  vPtr = reg->Get(addr);
  ASSERT_FALSE(vPtr);

  // verify registry retains object & releases it when done
  reg->Register(ptr);
  ptr.reset();
  vPtr.reset();
  vPtr = reg->Get(addr);
  ASSERT_TRUE(vPtr);
  vPtr.reset();
  vPtr = wPtr.lock();
  ASSERT_TRUE(vPtr);
  vPtr.reset();
  reg->Unregister(addr);
  vPtr = reg->Get(addr);
  ASSERT_FALSE(vPtr);
  vPtr = wPtr.lock();
  ASSERT_FALSE(vPtr);
}
