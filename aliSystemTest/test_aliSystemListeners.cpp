#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {
  using     LS   = aliSystem::Listeners<size_t>;
  using     L    = aliSystem::Listener<size_t>;
  namespace Util = aliSystem::Util;
}

TEST(aliSystemListeners, general) {
  size_t abc, ab, xyz;
  size_t ABC, AB, XYZ;
  size_t abcSum, abSum, xyzSum;
  abc = ab = xyz = 0;
  ABC = AB = XYZ = 0;
  abcSum = abSum = xyzSum = 0;
  LS::Ptr lPtr = LS::Create("onTest");
  L::Ptr  l1   = L::Create("abc", [&](size_t val) { ++abc; abcSum+=val; });
  L::Ptr  l2   = L::Create("ab",  [&](size_t val) { ++ab ; abSum +=val; });
  L::Ptr  l3   = L::Create("xyz", [&](size_t val) { ++xyz; xyzSum+=val; });
  lPtr->Register(l1, true);
  lPtr->Register(l2, false);
  lPtr->Register(l3, true);

  lPtr->Notify(10); ++ABC; ++AB; ++XYZ;
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);
  
  lPtr->Notify(100, []()->bool { return false; });
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);

  lPtr->Notify(1000, []()->bool { return true; }); ++ABC; ++AB; ++XYZ;
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);

  lPtr->Notify(10000, Util::GetMatchExactFn("abc")); ++ABC;
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);

  lPtr->Notify(100000, Util::GetMatchPrefixFn("ab")); ++ABC; ++AB;
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);

  l2.reset();
  l3.reset();
  lPtr->Notify(1000000); ++ABC; ++AB;
  ASSERT_EQ(abc, ABC);
  ASSERT_EQ(ab , AB );
  ASSERT_EQ(xyz, XYZ);

  ASSERT_EQ(abcSum, (size_t)1111010);
  ASSERT_EQ(abSum,  (size_t)1101010);
  ASSERT_EQ(xyzSum, (size_t)   1010);
}
