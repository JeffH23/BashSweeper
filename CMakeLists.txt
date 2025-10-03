cmake_minimum_required(VERSION 3.25)

project(MineSweeper)

set(CMAKE_BUILD_TYPE Debug)
add_executable(ms src/main.c)

# link the libm math library
target_link_libraries(ms PRIVATE m)

