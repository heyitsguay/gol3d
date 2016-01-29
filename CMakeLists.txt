cmake_minimum_required(VERSION 3.2)
project(gol3d)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -g -std=c++11 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++11")

set(SOURCE_FILES
        src/main.cpp
        src/load_shader.cpp
        src/load_bmp.cpp
        src/load_obj.cpp
        src/Cube.cpp
        src/World.cpp
        src/Camera.cpp
        src/IO.cpp
        src/Skybox.cpp
        src/User.cpp
        src/Application.cpp
    )
add_executable(gol3d ${SOURCE_FILES})

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(SOIL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${CMAKE_SOURCE_DIR}/include ${GLEW_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${SOIL_INCLUDE_DIRS})
target_link_libraries(gol3d ${GLEW_LIBRARIES} ${OpenGL_LIBRARIES} ${GLFW_STATIC_LIBRARIES} ${SOIL_LIBRARIES})
