cmake_minimum_required(VERSION 3.28.0)

project(RatVm)

file (GLOB_RECURSE sources CONFIGURE_DEPENDS test/*.c src/*.c src/*.h)

add_executable(ratvm ${sources})

target_include_directories(ratvm PRIVATE src)


