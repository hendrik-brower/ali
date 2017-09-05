#
# Lua bindings for libgit2
#
cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_C_FLAGS   "-Wall -Wextra -ggdb")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
option(CMAKE_CXX_EXTENSIONS "Build shared libraries (DLLs)." OFF)
project(APP C CXX)
find_package(Threads)
add_subdirectory(aliSystem)
add_subdirectory(aliSystemTest)
add_subdirectory(aliLuaCore)
add_subdirectory(aliLuaExt)
add_subdirectory(aliLuaExt2)
add_subdirectory(aliLuaExt3)
add_subdirectory(aliLuaTest)
add_subdirectory(app)
add_subdirectory(googletest)

#-Wall -ggdb


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxygen.in ${CMAKE_CURRENT_BINARY_DIR}/Doxygen @ONLY)
  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxygen
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
