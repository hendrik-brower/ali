add_executable(aliSystemTests
  aliSystemTest_main.cpp
  test_aliSystemBasicCodec.cpp
  test_aliSystemCodec.cpp
  test_aliSystemComponent.cpp
  test_aliSystemComponentRegistry.cpp
  test_aliSystemHold.cpp
  test_aliSystemListener.cpp
  test_aliSystemListeners.cpp
  test_aliSystemLogging.cpp
  test_aliSystemRegistry.cpp
  test_aliSystemStats.cpp
  test_aliSystemStatsGuard.cpp
  test_aliSystemThreadingPool.cpp
  test_aliSystemThreadingQueue.cpp
  test_aliSystemThreadingScheduler.cpp
  test_aliSystemThreadingSemaphore.cpp
  test_aliSystemThreadingWork.cpp
  test_aliSystemTime.cpp
  test_aliSystemUtil.cpp
  )
target_include_directories(aliSystemTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliSystemTests gtest)
target_link_libraries(aliSystemTests aliSystem)
target_link_libraries(aliSystemTests ${CMAKE_THREAD_LIBS_INIT})
set_property(TARGET aliSystemTests PROPERTY CXX_STANDARD 14)

