add_executable(app
  app1.cpp
  app2.cpp
  app3.cpp
  app4.cpp
  main.cpp
  )
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app LINK_PUBLIC
  aliSystem
  aliLuaCore
  aliLuaExt
  aliLuaExt2
  aliLuaExt3
  lua
  dl
  ${CMAKE_THREAD_LIBS_INIT}
  )
set_property(TARGET app PROPERTY CXX_STANDARD 14)


