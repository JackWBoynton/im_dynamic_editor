project(im_dynamic_editor_example)

find_package(glfw3 CONFIG REQUIRED)
add_executable(example ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_include_directories(example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../inc
                                          ${IMGUI_INCLUDES})
target_link_libraries(example PRIVATE im_dynamic_editor glfw ${IMGUI_LIBRARIES})
