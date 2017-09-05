add_library(aliLuaCore
  aliLuaCore_callTarget.cpp
  aliLuaCore.cpp
  aliLuaCore_deserialize.cpp
  aliLuaCore_exec.cpp
  aliLuaCore_functionMap.cpp
  aliLuaCore_functions.cpp
  aliLuaCore_future.cpp
  aliLuaCore_makeTableUtil.cpp
  aliLuaCore_module.cpp
  aliLuaCore_MT.cpp
  aliLuaCore_object.cpp
  aliLuaCore_serialize.cpp
  aliLuaCore_stackGuard.cpp
  aliLuaCore_staticObject.cpp
  aliLuaCore_stats.cpp
  aliLuaCore_table.cpp
  aliLuaCore_types.cpp
  aliLuaCore_util.cpp
  aliLuaCore_values.cpp
  )
target_include_directories(aliLuaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliLuaCore aliSystem)
set_property(TARGET aliLuaCore PROPERTY CXX_STANDARD 14)


