add_library(imgui
    imgui.cpp
    imgui_widgets.cpp
    imgui_impl_opengl3.cpp
    imgui_impl_glfw.cpp
    imgui_draw.cpp
    imgui_demo.cpp
)

target_include_directories(imgui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(imgui
    PUBLIC glad glfw
)

target_compile_definitions(imgui
    PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD
)