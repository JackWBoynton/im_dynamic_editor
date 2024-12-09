cmake_minimum_required(VERSION 3.15)

project(im_dynamic_editor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

macro(addIncludesFromLibrary target library)
  get_target_property(library_include_dirs ${library} INTERFACE_INCLUDE_DIRECTORIES)
  if (NOT library_include_dirs)
    message(FATAL_ERROR "Library ${library} does not have INTERFACE_INCLUDE_DIRECTORIES")
  endif()
  target_include_directories(${target} PUBLIC ${library_include_dirs})
  message(STATUS "Adding include directories from ${library}: ${library_include_dirs}")
endmacro()

if(NOT DEFINED IM_DYNAMIC_EDITOR_STANDALONE_PROJECT)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IM_DYNAMIC_EDITOR_STANDALONE_PROJECT ON)
  else()
    set(IM_DYNAMIC_EDITOR_STANDALONE_PROJECT OFF)
  endif()
endif()

# CMake options
option(IM_DYNAMIC_EDITOR_EXAMPLE "Build example" ${IM_DYNAMIC_EDITOR_STANDALONE_PROJECT})
option(IM_DYNAMIC_EDITOR_USE_BUNDLED_LIBS "Use the bundled dependencies" ON)
option(IM_DYNAMIC_EDITOR_ADD_BUNDLED_WIDGETS "Register all default bundled widgets" OFF)

# Create the main library target
add_library(im_dynamic_editor_lib)

if(IM_DYNAMIC_EDITOR_USE_BUNDLED_LIBS)
  # Add bundled imgui and dependencies
  add_subdirectory(external/imgui_bundle)
  add_subdirectory(external/nlohmann_json)

  set(IMGUI_INCLUDES imgui_all_includes ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann_json/single_include)
  set(IMGUI_LIBRARIES imgui_imgui imgui_implot imnodes imgrid imgui_custom nlohmann_json::nlohmann_json)
else()
if (NOT DEFINED IM_DYNAMIC_EDITOR_IMGUI_TARGET_NAME OR NOT DEFINED IM_DYNAMIC_EDITOR_IMGRID_TARGET_NAME OR NOT DEFINED IM_DYNAMIC_EDITOR_IMNODES_TARGET_NAME)
  find_package(imgui CONFIG REQUIRED)
else()
  addIncludesFromLibrary(im_dynamic_editor_lib imgrid)
  target_link_libraries(im_dynamic_editor_lib PRIVATE ${IM_DYNAMIC_EDITOR_IMGUI_TARGET_NAME} ${IM_DYNAMIC_EDITOR_IMGRID_TARGET_NAME} ${IM_DYNAMIC_EDITOR_IMNODES_TARGET_NAME} implot)
endif()
endif()


if(IM_DYNAMIC_EDITOR_ADD_BUNDLED_WIDGETS)
  # Add the widgets static library
  add_library(widgets STATIC)

  # Collect all source files in the widgets folder
  file(GLOB_RECURSE WIDGET_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/widgets/src/*.cpp")

  # Add include directories and link dependencies
  target_include_directories(widgets PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/widgets/inc" "${CMAKE_CURRENT_SOURCE_DIR}/inc" ${IMGUI_INCLUDES}
                                            ${CMAKE_CURRENT_SOURCE_DIR}/external/codicons/)

  target_link_libraries(widgets PRIVATE ${IMGUI_LIBRARIES})
  target_sources(widgets PRIVATE ${WIDGET_SOURCES})

  # Include widget objects in the main library
  set(LIB_LIBS ${LIB_LIBS} $<TARGET_OBJECTS:widgets>)
endif()

# Collect the main source files for the library
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources(im_dynamic_editor_lib PRIVATE ${SOURCES})

# Include directories and link libraries
target_include_directories(im_dynamic_editor_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/external/codicons/)
target_include_directories(im_dynamic_editor_lib PRIVATE ${IMGUI_INCLUDES} 
                                                        ${NLOHMANN_JSON_INCLUDES})

target_link_libraries(im_dynamic_editor_lib PRIVATE ${LIB_LIBS} ${IMGUI_INCLUDES} ${NLOHMANN_JSON_LIBRARIES})

# If building the example
if(IM_DYNAMIC_EDITOR_EXAMPLE)
  add_subdirectory(example)
endif()
