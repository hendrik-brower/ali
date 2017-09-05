#ifndef INCLUDED_ALI_TEST_UTIL
#define INCLUDED_ALI_TEST_UTIL

#include <aliSystem.hpp>
#include <aliLuaCore.hpp>
#include <functional>
#include <memory>
#include <string>

struct lua_state;
namespace aliLuaTest {
  
  struct Util {
    using BPtr    = std::shared_ptr<bool>;
    using IPtr    = std::shared_ptr<int>;
    using DPtr    = std::shared_ptr<double>;
    using SPtr    = std::shared_ptr<std::string>;
    using BWPtr   = std::weak_ptr<bool>;
    using IWPtr   = std::weak_ptr<int>;
    using DWPtr   = std::weak_ptr<double>;
    using LPtr    = std::shared_ptr<lua_State>;
    using IVec    = std::vector<int>;
    using IVecPtr = std::shared_ptr<IVec>;
    static LPtr GetL();
    static void Wait(const aliLuaCore::Exec::Ptr &exec,
		     const aliLuaCore::Future::Ptr &fPtr=nullptr);
    static bool DidThrow(const std::function<void()> &fn);
    static void RegisterInitFini(aliSystem::ComponentRegistry &cr);
  };
}

#endif
  
