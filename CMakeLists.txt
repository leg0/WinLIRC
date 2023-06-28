cmake_minimum_required(VERSION 3.20)

set(VCPKG_MANIFEST_MODE ON)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(WinLIRC VERSION 1.0.0)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set (WL_PLATFORM ${VCPKG_TARGET_TRIPLET})

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


add_compile_options(/std:c++latest /MP)

add_subdirectory(winlirc)
add_subdirectory(DLL)
add_subdirectory(Tools)



install(
    DIRECTORY configs
    DESTINATION .
    PATTERN ".git" EXCLUDE
    )

# Set CPack variables for the installer
set(CPACK_PACKAGE_VENDOR "open source")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "winlirc infrared receiver and sender")

set(CPACK_GENERATOR WIX)
set(CPACK_WIX_UPGRADE_GUID "772e8de1-5874-4bed-b53c-fdf25dfd228b")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE.txt")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_LIST_DIR}/winlirc/lirc.ico")
set(CPACK_MONOLITHIC_INSTALL FALSE)
set(CPACK_SOURCE_IGNORE_FILES
    \\\\.git/
    .git/
    \\\\.swp$
    \\\\.orig$
)
include(CPack)

