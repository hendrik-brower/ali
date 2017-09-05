#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {

  using SVVec     = std::vector<std::string>;
  using SVPtr     = std::shared_ptr<SVVec>;
  using CR        = aliSystem::ComponentRegistry;
  using Component = aliSystem::Component;
  using CPtr      = Component::Ptr;
  using CPVec     = std::vector<CPtr>;
  using IVec      = std::vector<size_t>;

  Component::Fn GetFn(const SVPtr       &svPtr,
		      const std::string &str) {
    THROW_IF(!svPtr, "svPtr cannot be null");
    return [=]() {
      svPtr->push_back(str);
    };
  }
  bool OrderByName(const CPtr &a, const CPtr &b) {
    THROW_IF(!a || !b, "Cannot compare with a or b equal to ull");
    return a->Name()<b->Name();
  }

  void Config(const CPVec &vec, size_t item, size_t depItem) {
    THROW_IF(item   >=vec.size(), "bad item index");
    THROW_IF(depItem>=vec.size(), "bad depItem index");
    vec[item]->AddDependency(vec[depItem]->Name());
  }
}

TEST(ComponentRegistry, general) {
  CR       cr;
  CR::Vec  comp;
  CR::SSet sSet;
  SVPtr    svPtr(new SVVec);
  ASSERT_FALSE(cr.HasInit());
  ASSERT_FALSE(cr.HasFini());
  CPtr a1 = cr.Register("a1");
  CPtr a2 = cr.Register("a2", GetFn(svPtr, "a2-init"), GetFn(svPtr,"a2-fini"));
  CPtr a3 = Component::Create("a3", GetFn(svPtr, "a3-init"), GetFn(svPtr,"a3-fini"));
  CPtr a3x = cr.Register(a3);
  ASSERT_EQ(a3.get(),a3x.get());
  ASSERT_FALSE(cr.HasInit());
  ASSERT_FALSE(cr.HasFini());
  cr.GetComponents(comp);
  ASSERT_EQ(comp.size(), 3u);
  std::sort(comp.begin(), comp.end(), &OrderByName);
  ASSERT_EQ(comp[0]->Name(), "a1");
  ASSERT_EQ(comp[1]->Name(), "a2");
  ASSERT_EQ(comp[2]->Name(), "a3");
  cr.GetSrcSet(sSet);
  ASSERT_EQ(sSet.size(),3u);
  ASSERT_TRUE(sSet.find("a1")!=sSet.end());
  ASSERT_TRUE(sSet.find("a2")!=sSet.end());
  ASSERT_TRUE(sSet.find("a3")!=sSet.end());
  sSet.clear();
  cr.GetDepSet(sSet);
  ASSERT_EQ(sSet.size(), 0u);
  a1->AddDependency("a2");
  cr.GetDepSet(sSet);
  ASSERT_EQ(sSet.size(), 1u);
  ASSERT_TRUE(sSet.find("a2")!=sSet.end());

  //
  // basic check of init behavior
  ASSERT_FALSE(a1->IsFrozen());
  ASSERT_FALSE(a2->IsFrozen());
  ASSERT_FALSE(a3->IsFrozen());
  cr.Init();
  ASSERT_TRUE(cr.HasInit());
  ASSERT_FALSE(cr.HasFini());
  ASSERT_TRUE(a1->IsFrozen());
  ASSERT_TRUE(a2->IsFrozen());
  ASSERT_TRUE(a3->IsFrozen());

  //
  // basic check of fini behavior
  cr.Fini();
  ASSERT_TRUE(cr.HasInit());
  ASSERT_TRUE(cr.HasFini());

  //
  // Check of execution ordering
  ASSERT_EQ(svPtr->size(), 4u);
  ASSERT_EQ(svPtr->operator[](0), "a2-init");
  ASSERT_EQ(svPtr->operator[](1), "a3-init");
  ASSERT_EQ(svPtr->operator[](2), "a3-fini");
  ASSERT_EQ(svPtr->operator[](3), "a2-fini");
}


TEST(ComponentRegistry, basicDepProcessing) {
  CR    cr;
  CPVec vec;
  SVPtr svPtr(new SVVec);
  IVec  order;
  for (int i=0;i<10;++i) {
    char name[25];
    char init[25];
    char fini[25];
    snprintf(name, sizeof(name), "a%02i", i);
    snprintf(init, sizeof(init), "init-a%02i", i);
    snprintf(fini, sizeof(fini), "fini-a%02i", i);
    CPtr ptr = cr.Register(init, GetFn(svPtr, init), GetFn(svPtr,fini));
    vec.push_back(ptr);
  }
  //
  // configure dependencies:
  //   a0 -> a1, a2
  //   a1 -> a2, a3
  //   a2 -> a3, a4
  //   a3 -> << nothing >>
  //   a4 -> << nothing >>
  //   a5 -> a6, a4
  //   a6 -> a0
  //   a7 -> a1
  //   a8 -> a2
  //   a9 -> a3
  // expected order:
  //   a3, a4, a2, a1, a0, a6, a5, a7, a8, a9
  Config(vec, 0, 1);
  Config(vec, 0, 2);
  Config(vec, 1, 2);
  Config(vec, 1, 3);
  Config(vec, 2, 3);
  Config(vec, 2, 4);
  Config(vec, 5, 6);
  Config(vec, 5, 4);
  Config(vec, 6, 0);
  Config(vec, 7, 1);
  Config(vec, 8, 2);
  Config(vec, 9, 3);
  cr.Init();
  cr.Fini();
  order.push_back(3);
  order.push_back(4);
  order.push_back(2);
  order.push_back(1);
  order.push_back(0);
  order.push_back(6);
  order.push_back(5);
  order.push_back(7);
  order.push_back(8);
  order.push_back(9);
  ASSERT_EQ(svPtr->size(), 20u);
  for (IVec::iterator it=order.begin(); it!=order.end(); ++it) {
    char init[25];
    snprintf(init, sizeof(init), "init-a%02i", (int)*it);
    ASSERT_STREQ(svPtr->operator[](it-order.begin()).c_str(), init);
  }
  for (IVec::iterator it=order.begin(); it!=order.end(); ++it) {
    char fini[25];
    snprintf(fini, sizeof(fini), "fini-a%02i", (int)*it);
    ASSERT_STREQ(svPtr->operator[](20-1-(it-order.begin())).c_str(), fini);
  }
}

TEST(ComponentRegistry, circularDepThrows) {
  CR    cr;
  CPVec vec;
  SVPtr svPtr(new SVVec);
  for (int i=0;i<4;++i) {
    char name[25];
    char init[25];
    char fini[25];
    snprintf(name, sizeof(name), "a%02i", i);
    snprintf(init, sizeof(init), "init-a%02i", i);
    snprintf(fini, sizeof(fini), "fini-a%02i", i);
    CPtr ptr = cr.Register(init, GetFn(svPtr, init), GetFn(svPtr,fini));
    vec.push_back(ptr);
  }
  Config(vec, 0, 1);
  Config(vec, 1, 2);
  Config(vec, 2, 3);
  Config(vec, 3, 0);
  ASSERT_ANY_THROW(cr.Init());
}

TEST(ComponentRegistry, undefinedThrows) {
  CR    cr;
  SVPtr svPtr(new SVVec);
  CPtr ptr = cr.Register("a1", GetFn(svPtr, "init"), GetFn(svPtr,"fini"));
  ptr->AddDependency("aXXX");
  ASSERT_ANY_THROW(cr.Init());
}
