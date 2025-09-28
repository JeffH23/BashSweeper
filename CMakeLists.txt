cmake_minimum_required(VERSION 3.25)

project(MineSweeper)

add_executable(ms src/main.c)

# link the libm math library
target_link_libraries(ms PRIVATE m)

