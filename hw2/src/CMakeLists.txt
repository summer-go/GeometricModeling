set(PYTHON /Users/baidu/opt/anaconda3/envs/python36)

include_directories(${PYTHON}/include)

add_executable(${PROJECT_NAME}
    main.cpp
    MathUtil.cpp
    RbfNetwork.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/rbf_nn.py
    ${CMAKE_CURRENT_BINARY_DIR}/mytest.py
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ${PYTHON}/lib/libpython3.6m.dylib
        )

target_link_libraries(${PROJECT_NAME}
    PRIVATE imgui Eigen
)

target_compile_features(${PROJECT_NAME}
    PRIVATE cxx_std_17
)

message(STATUS "before add custom_command")

#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rbf_nn.py
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rbf_nn.py
#    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/rbf_nn.py ${CMAKE_CURRENT_BINARY_DIR}/rbf_nn.py
#)
#
#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mytest.py
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mytest.py
#    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/mytest.py ${CMAKE_CURRENT_BINARY_DIR}/mytest.py
#)

file(COPY mytest.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY rbf_nn.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "after add custom_command")
