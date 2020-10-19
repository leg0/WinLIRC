cmake_minimum_required(VERSION 3.9)

project(WinLIRC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

if (MSVC)
	set (WL_PLATFORM ${CMAKE_VS_PLATFORM_NAME})
else()
	set (WL_PLATFORM ${CMAKE_GENERATOR_PLATFORM})
endif()

enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON) 


macro(configure_plugin plugin_name)
  set_target_properties(${plugin_name} PROPERTIES  FOLDER "Plugins")
  target_link_libraries(${plugin_name} PRIVATE winlirc)
  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( ${plugin_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG}/plugins )
  endforeach()

  install(TARGETS ${plugin_name}
	DESTINATION plugins)
endmacro()


macro(configure_tool tool_name)
  set_target_properties(${tool_name} PROPERTIES  FOLDER "Tools")

  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( ${tool_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
  endforeach()

  install(TARGETS ${tool_name}
          DESTINATION .)
endmacro()


if (MSVC)
  add_compile_options(/std:c++latest /MP)
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

add_subdirectory(winlirc)

add_subdirectory(DLL)

add_subdirectory(IRRecord)
add_subdirectory(Tools)

install(DIRECTORY configs DESTINATION .)
