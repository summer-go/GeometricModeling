add_executable(game102_hw6 main.cpp)
target_link_libraries(game102_hw6 pmp_pupa pmp_vis)

if (EMSCRIPTEN)
    set_target_properties(game102_hw6 PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pmp
            LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/extern/pmp-library/src/apps/data/shell.html --preload-file ${PROJECT_SOURCE_DIR}/data/@/"
            )
endif ()
