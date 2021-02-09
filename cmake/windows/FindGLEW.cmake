# - Config file for the glew package
# It defines the following variables
#   GLEW_INCLUDE_DIR, the path where GLEW headers are located
#   GLEW_LIBRARY_DIR, folder in which the GLEW library is located
#   GLEW_LIBRARY, library to link against to use GLEW

set(GLEW_INCLUDE_DIR "C:/MinGW/include")
set(GLEW_LIBRARY_DIR "C:/MinGW/lib")

find_library(GLEW_LIBRARIES NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib HINTS ${GLEW_LIBRARY_DIR})
