add_library(aliLuaExt2
  aliLuaExt2.cpp
  aliLuaExt2_action.cpp
  )
target_include_directories(aliLuaExt2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliLuaExt2
  aliSystem
  aliLuaCore)
set_property(TARGET aliLuaExt2 PROPERTY CXX_STANDARD 14)


