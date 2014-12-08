cmake_minimum_required(VERSION 2.8)

project(WinLIRC)

enable_testing()
# http://www.cmake.org/cmake/help/v3.0/module/FindGTest.html
find_package(GTest)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) 


macro(configure_plugin plugin_name)
  set_target_properties(${plugin_name} PROPERTIES  FOLDER "DLL")

  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( ${plugin_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG}/plugins )
  endforeach()
endmacro()


macro(configure_tool tool_name)
  set_target_properties(${tool_name} PROPERTIES  FOLDER "Tools")

  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( ${tool_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
  endforeach()
endmacro()


add_subdirectory(winlirc)

add_subdirectory(DLL)

add_subdirectory(IRRecord)
add_subdirectory(Tools)
