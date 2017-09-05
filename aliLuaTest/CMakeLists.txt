add_executable(aliLuaTests
  aliLuaTest_exec.cpp
  aliLuaTest_main.cpp
  aliLuaTest_util.cpp
  test_aliLuaCore_callTarget.cpp	       
  test_aliLuaCore_deserialization.cpp    
  test_aliLuaCore_exec.cpp	       
  test_aliLuaCore_functionMap.cpp	       
  test_aliLuaCore_functions.cpp	       
  test_aliLuaCore_future.cpp	       
  test_aliLuaCore_makeTableUtil.cpp      
  test_aliLuaCore_module.cpp	         
  test_aliLuaCore_MT.cpp		       
  test_aliLuaCore_object.cpp	       
  test_aliLuaCore_serialization.cpp      
  test_aliLuaCore_stackGuard.cpp	       
  test_aliLuaCore_staticObject.cpp       
  test_aliLuaCore_table.cpp	       
  test_aliLuaCore_types.cpp	       
  test_aliLuaCore_util.cpp	       
  test_aliLuaCore_values.cpp	       
  test_aliLuaExt_codec.cpp
  test_aliLuaExt_execEngine.cpp	       
  test_aliLuaExt_execEngineWork.cpp      
  test_aliLuaExt_execPool.cpp	       
  test_aliLuaExt_execPoolItem.cpp	       
  test_aliLuaExt_functionTarget.cpp      
  test_aliLuaExt_hold.cpp		       
  test_aliLuaExt_IO.cpp		       
  test_aliLuaExt_IOOptions.cpp	       
  test_aliLuaExt_objectTarget.cpp	       
  test_aliLuaExt_queue.cpp	       
  test_aliLuaExt_stats.cpp	       
  test_aliLuaExt_threading.cpp	       
  test_aliLuaExt2_action.cpp	       
  test_aliLuaExt3_externalMT.cpp	       
  test_aliLuaExt3_externalObject.cpp     
  test_performance.cpp                   
  )
target_include_directories(aliLuaTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(aliLuaTests LINK_PUBLIC gtest
  # gtest_main
  aliSystem
  aliLuaCore
  aliLuaExt
  aliLuaExt2
  aliLuaExt3
  ${CMAKE_THREAD_LIBS_INIT}
  lua
  dl)
set_property(TARGET aliLuaTests PROPERTY CXX_STANDARD 14)


