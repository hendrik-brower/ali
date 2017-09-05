add_library(aliSystem
  aliSystem.cpp

  aliSystem_basicCodec.cpp
  aliSystem_codec.cpp
  aliSystem_codecSerialize.cpp
  aliSystem_codecSerializer.cpp
  aliSystem_codecDeserialize.cpp
  aliSystem_codecDeserializer.cpp
  aliSystem_component.cpp
  aliSystem_componentRegistry.cpp
  aliSystem_hold.cpp
  aliSystem_listener.cpp
  aliSystem_listeners.cpp
  aliSystem_logging.cpp
  aliSystem_registry.cpp
  aliSystem_stats.cpp
  aliSystem_statsGuard.cpp
  aliSystem_threading.cpp
  aliSystem_threadingPool.cpp
  aliSystem_threadingQueue.cpp
  aliSystem_threadingScheduler.cpp
  aliSystem_threadingSemaphore.cpp
  aliSystem_threadingWork.cpp
  aliSystem_time.cpp
  aliSystem_util.cpp
  )

target_include_directories(aliSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET aliSystem PROPERTY CXX_STANDARD 14)


