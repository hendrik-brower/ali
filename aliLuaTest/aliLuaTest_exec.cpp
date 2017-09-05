#include <aliLuaTest_exec.hpp>
#include <aliLuaExt.hpp>

namespace aliLuaTest {

  void Exec::SetUp() {
    pool = aliSystem::Threading::Pool::Create("test pool",2);
    exec = aliLuaExt::ExecEngine::Create("test exec", pool);
  }
  void Exec::TearDown() {
    exec.reset();
    pool.reset();
  }


}
