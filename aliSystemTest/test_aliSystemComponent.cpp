#include "gtest/gtest.h"
#include <aliSystem.hpp>

namespace {

  using SVVec     = std::vector<std::string>;
  using SVPtr     = std::shared_ptr<SVVec>;
  using Component = aliSystem::Component;

  Component::Fn GetFn(const SVPtr       &svPtr,
		      const std::string &str) {
    THROW_IF(!svPtr, "svPtr cannot be null");
    return [=]() {
      svPtr->push_back(str);
    };
  }
  
}

TEST(aliSystemComponentTest, general) {  
  SVPtr           svPtr(new SVVec);
  Component::SSet sSet;
  Component::Ptr  ptr = Component::Create("a1",
					  GetFn(svPtr, "a1-init"),
					  GetFn(svPtr, "a1-fini"));
  ASSERT_TRUE(ptr);
  ptr->AddDependency("a2");
  ptr->AddDependency("a3");
  Component::Fn  init = ptr->GetInitFn();
  Component::Fn  fini = ptr->GetFiniFn();
  ASSERT_STREQ("a1", ptr->Name().c_str());
  ASSERT_FALSE(ptr->IsFrozen());
  ASSERT_FALSE(ptr->WasInit());
  ASSERT_FALSE(ptr->WasFini());
  ASSERT_EQ(ptr->NumDep(), 2u);
  ptr->GetDependencies(sSet);
  ASSERT_EQ(sSet.size(),2u);
  sSet.clear();
  sSet.insert("ax");
  ptr->GetDependencies(sSet);
  ASSERT_EQ(sSet.size(),3u) << "verify sSet is not cleared";
  ASSERT_TRUE(sSet.find("ax")!=sSet.end());
  ASSERT_TRUE(sSet.find("a2")!=sSet.end());
  ASSERT_TRUE(sSet.find("a3")!=sSet.end());
  ptr->Freeze();
  ASSERT_TRUE(ptr->IsFrozen());
  ASSERT_ANY_THROW(ptr->AddDependency("a4"));
  ASSERT_EQ(ptr->NumDep(),2u) << "verify no dependency set is fixed once frozen";
  init();
  ASSERT_TRUE(ptr->WasInit());
  ASSERT_FALSE(ptr->WasFini());
  ASSERT_EQ(svPtr->size(),1u);
  ASSERT_STREQ(svPtr->operator[](0).c_str(),"a1-init");  
  fini();
  ASSERT_TRUE(ptr->WasInit());
  ASSERT_TRUE(ptr->WasFini());
  ASSERT_EQ(svPtr->size(),2u);
  ASSERT_STREQ(svPtr->operator[](0).c_str(),"a1-init");  
  ASSERT_STREQ(svPtr->operator[](1).c_str(),"a1-fini");  
}
