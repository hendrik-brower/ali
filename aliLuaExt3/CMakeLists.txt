add_library(aliLuaExt3
  aliLuaExt3.cpp
  aliLuaExt3_externalMT.cpp
  aliLuaExt3_externalObject.cpp
  )
target_include_directories(aliLuaExt3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliLuaExt3
  aliSystem
  aliLuaCore
  aliLuaExt)
set_property(TARGET aliLuaExt3 PROPERTY CXX_STANDARD 14)


