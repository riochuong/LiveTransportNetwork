cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(ZLIB REQUIRED)
find_package(Boost 1.81.0 REQUIRED COMPONENTS system)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${SOURCES})

target_compile_features(network-monitor
	PRIVATE
		cxx_std_17
)
target_link_libraries(network-monitor 
	PRIVATE
		Boost::system
)
