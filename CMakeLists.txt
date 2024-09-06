cmake_minimum_required(VERSION 3.15)

project(im_dynamic_editor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT DEFINED IM_DYNAMIC_EDITOR_STANDALONE_PROJECT)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(IM_DYNAMIC_EDITOR_STANDALONE_PROJECT ON)
    else()
        set(IM_DYNAMIC_EDITOR_STANDALONE_PROJECT OFF)
    endif ()
endif()

option(IM_DYNAMIC_EDITOR_EXAMPLE "Build example" ${IM_DYNAMIC_EDITOR_STANDALONE_PROJECT})
option(IM_DYNAMIC_EDITOR_USE_BUNDLED_LIBS "Use the bundled dependencies" ON)

if (IM_DYNAMIC_EDITOR_USE_BUNDLED_LIBS)
  add_subdirectory(external/imgui_bundle)
  set(IMGUI_INCLUDES imgui_all_includes)
else()
  # TODO:
endif()

add_library(im_dynamic_editor)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
target_sources(im_dynamic_editor PRIVATE ${SOURCES})
target_include_directories(im_dynamic_editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${IMGUI_INCLUDES})
target_link_libraries(im_dynamic_editor PUBLIC ${IMGUI_LIBRARIES})

if(IM_DYNAMIC_EDITOR_EXAMPLE)
  add_subdirectory(example)
endif()
