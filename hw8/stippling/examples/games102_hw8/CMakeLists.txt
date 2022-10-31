get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

include_directories(${PROJECT_SOURCE_DIR}/extern/voronoi/src)
include_directories(${PROJECT_SOURCE_DIR}/extern/pmp-library/external/stb_image)
add_executable(${PROJECT_NAME} VoronoiApp.cpp ImageSampler.cpp)
target_link_libraries(${PROJECT_NAME} pmp_vis)

configure_file(run.py ${CMAKE_BINARY_DIR}/run.py COPYONLY)

#add_executable(${PROJECT_NAME}_varing varing_density_voronoi.cpp)


#if (EMSCRIPTEN)
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
#            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
#            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
#            LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/extern/pmp-library/src/apps/data/shell.html --preload-file ${PROJECT_SOURCE_DIR}/data/@/"
#            )
#endif ()

