cmake_minimum_required(VERSION 3.0)
project(opengl-imgui-sample CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(imgui CONFIG)
find_package(glfw3 CONFIG) # 
find_package(glfw CONFIG)
find_package(glew CONFIG)
find_package(fmt CONFIG)
find_package(glm CONFIG)
find_package(stb CONFIG)
find_package(tinyobjloader CONFIG)
find_package(spdlog CONFIG)

set(_src_root_path "${PROJECT_SOURCE_DIR}")

add_executable( opengl-imgui-sample
  main.cpp
  obj_model.cpp
  obj_model.h
  torus.cpp
  torus.h
  opengl_shader.cpp
  opengl_shader.h
  bindings/imgui_impl_glfw.cpp
  bindings/imgui_impl_opengl3.cpp
  bindings/imgui_impl_glfw.h
  bindings/imgui_impl_opengl3.h
  assets/simple-shader.vs
  assets/simple-shader.fs
  assets/model.vs
  assets/model.fs
  )

add_custom_command(TARGET opengl-imgui-sample
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/simple-shader.fs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/model.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/model.fs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/torus.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/torus.fs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:opengl-imgui-sample> ${PROJECT_BINARY_DIR}
    )

set_target_properties(opengl-imgui-sample PROPERTIES LINKER_LANGUAGE CXX)
set_property(TARGET opengl-imgui-sample PROPERTY CXX_STANDARD 17)

target_compile_definitions(opengl-imgui-sample PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(opengl-imgui-sample imgui::imgui GLEW::glew_s glfw::glfw fmt::fmt glm::glm stb::stb tinyobjloader::tinyobjloader spdlog::spdlog)
