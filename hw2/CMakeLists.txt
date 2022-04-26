cmake_minimum_required(VERSION 3.15)

project(GAMES102-hw2)

include(cmake/CPM.cmake)

#find_package(Python3 COMPONENTS Development)
#if(Python3_FOUND)
#    message(STATUS ”CURL library found” ${Python3_INCLUDE_DIRS})
#else(Python3_FOUND)
#    message(FATAL_ERROR ”CURL library not found”)
#endif(Python3_FOUND)

CPMAddPackage(
    NAME GLFW
    GITHUB_REPOSITORY glfw/glfw
    GIT_TAG 3.3.2
    OPTIONS
    "GLFW_BUILD_TESTS OFF"
    "GLFW_BUILD_EXAMPLES OFF"
    "GLFW_BULID_DOCS OFF"
)

CPMAddPackage(
    NAME Eigen
    VERSION 3.3.7
    URL https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.gz
    DOWNLOAD_ONLY YES
)
if(Eigen_ADDED)
    add_library(Eigen INTERFACE IMPORTED)
    target_include_directories(Eigen INTERFACE ${Eigen_SOURCE_DIR})
endif()

add_subdirectory(external)
add_subdirectory(src)