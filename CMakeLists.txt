cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

set(CMAKE_VERBOSE_MAKEFILE ON)

enable_testing()

add_subdirectory(tests)
add_subdirectory(src)