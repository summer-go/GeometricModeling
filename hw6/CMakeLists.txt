project(pmp-games102)
cmake_minimum_required(VERSION 3.5.2)
#cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 17 )
set(OpenGL_GL_PREFERENCE GLVND)

#cmake_policy(SET CMP0048 NEW)
#if(${CMAKE_VERSION} VERSION_GREATER "3.11.0")
#    cmake_policy(SET CMP0072 OLD)
#endif()

# set output paths
set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# compile PMP library
set(PMP_BUILD_APPS     OFF CACHE BOOL "")
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "")
set(PMP_BUILD_TESTS    OFF CACHE BOOL "")
set(PMP_BUILD_DOCS     OFF CACHE BOOL "")
add_subdirectory(extern/pmp-library)

# add include directories

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/dualquat)
include_directories(${PROJECT_SOURCE_DIR}/extern/entityx)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/src)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/external/imgui)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/external/eigen)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/external/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/external/glew/include)

# set default compiler flags
if((UNIX OR APPLE) AND CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter")
endif()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MATH_DEFINES -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS")
endif()

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -s ALLOW_MEMORY_GROWTH=1 -s USE_GLFW=3 -s WASM=0 -s USE_WEBGL2=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# make a release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()


if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
endif()

#set(ENTITYX_BUILD_TESTING OFF CACHE BOOL "Enable building of tests.")
#add_subdirectory(extern/entityx)
#add_subdirectory(tests)
add_subdirectory(src)

