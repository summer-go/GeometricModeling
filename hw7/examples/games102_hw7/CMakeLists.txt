get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${PROJECT_NAME} main.cpp harmonic.cpp)
target_link_libraries(${PROJECT_NAME} pmp_vis)

if (EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/extern/pmp-library/src/apps/data/shell.html --preload-file ${PROJECT_SOURCE_DIR}/data/@/"
            )
endif ()

