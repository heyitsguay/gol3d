cmake_minimum_required(VERSION 3.5)
project(gol3d)

set(OpenGL_GL_PREFERENCE GLVND)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#
#if(WIN32)
#    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/windows")
#endif(WIN32)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++20 -O3 -funroll-loops -ffinite-math-only -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++20")

set(SOURCE_FILES
        src/main.cpp
        src/load_shader.cpp
        src/load_obj.cpp
        src/Cube.cpp
        src/Camera.cpp
        src/IO.cpp
        src/Skybox.cpp
        src/User.cpp
        src/Application.cpp
        src/Object.cpp
        src/CellularAutomaton.cpp
        src/GeneralizedCellularAutomaton.cpp
        src/World.cpp
        src/utils.cpp
        src/Rule.cpp)
add_executable(gol3d ${SOURCE_FILES})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SOIL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/nlohmann
        ${GLEW_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${SOIL_INCLUDE_DIRS}
)

target_link_libraries(gol3d
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${GLFW_STATIC_LIBRARIES}
        ${SOIL_LIBRARIES}
)