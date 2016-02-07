# - Config file for the glfw package
# It defines the following variables
#   GLFW_INCLUDE_DIR, the path where GLFW headers are located
#   GLFW_LIBRARY_DIR, folder in which the GLFW library is located
#   GLFW_LIBRARY, library to link against to use GLFW

set(GLFW_INCLUDE_DIR "C:/Program Files (x86)/GLFW/include")
set(GLFW_LIBRARY_DIR "C:/Program Files (x86)/GLFW/lib")

find_library(GLFW_LIBRARY "glfw3" HINTS ${GLFW_LIBRARY_DIR})
