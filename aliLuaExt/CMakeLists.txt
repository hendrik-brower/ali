add_library(aliLuaExt
  aliLuaExt.cpp
  aliLuaExt_codec.cpp
  aliLuaExt_execEngine.cpp
  aliLuaExt_execEngineWork.cpp
  aliLuaExt_execPool.cpp
  aliLuaExt_execPoolItem.cpp
  aliLuaExt_functionTarget.cpp
  aliLuaExt_hold.cpp
  aliLuaExt_IO.cpp
  aliLuaExt_IOOptions.cpp
  aliLuaExt_objectTarget.cpp
  aliLuaExt_queue.cpp
  aliLuaExt_threading.cpp
  )

target_include_directories(aliLuaExt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliLuaExt aliSystem)
target_link_libraries(aliLuaExt aliLuaCore)
set_property(TARGET aliLuaExt PROPERTY CXX_STANDARD 14)


