cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(network-monitor)

# this is an important line to make sure CMAKE can see conan downloaded
# package properly
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(Boost 1.81.0 REQUIRED COMPONENTS system)
find_package(spdlog 1.11.0 REQUIRED)
find_package(fmt REQUIRED)

# not sure why find_package cannot include spdlog and fmt headers here 
include_directories("${spdlog_INCLUDE_DIR}")
include_directories("${fmt_INCLUDE_DIR}")

# need to include this so spdlog wont 
# need it  #include <spdlog/fmt/bundled/core.h> which apparently is not part of the package 
add_compile_definitions(SPDLOG_FMT_EXTERNAL)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp")

add_executable(network-monitor ${SOURCES})

target_include_directories(network-monitor
    PRIVATE
    src
)

target_compile_features(network-monitor
	PRIVATE
		cxx_std_17
)
target_link_libraries(network-monitor 
	PRIVATE
		spdlog::spdlog
		Boost::system
)
