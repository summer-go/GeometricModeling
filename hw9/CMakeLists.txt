cmake_minimum_required(VERSION 3.1)
project(HW9)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# libigl
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)

find_package(LIBIGL REQUIRED QUIET)
find_package(OpenMesh REQUIRED)

# Add your project files
file(GLOB SRCFILES *.cpp)
add_executable(${PROJECT_NAME} ${SRCFILES} Base.hpp Base.cpp SurfaceSimplifacation.cpp SurfaceSimplification.h)
target_link_libraries(${PROJECT_NAME} igl::core igl::opengl_glfw)
target_link_libraries(${PROJECT_NAME} ${OpenMesh_LIBS} OpenMeshCore OpenMeshTools)
