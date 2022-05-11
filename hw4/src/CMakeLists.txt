add_executable(${PROJECT_NAME}
        main.cpp
        myapp.cpp
        fitcurve.cpp
        MathUtil.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE imgui Eigen
)

target_compile_features(${PROJECT_NAME}
    PRIVATE cxx_std_17
)