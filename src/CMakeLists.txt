cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

find_package(Boost 1.81.0 REQUIRED COMPONENTS system)
find_package(spdlog 1.10.0 REQUIRED)
find_package(fmt 8.1.1 REQUIRED)


# need to specify include directories here 
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${spdlog_INCLUDE_DIRS}")
include_directories("${fmt_INCLUDE_DIR}")

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/websocket-client.cpp")

add_library(network-monitor ${SOURCES})

target_include_directories(network-monitor
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../inc"
)

target_compile_features(network-monitor
	PRIVATE
	cxx_std_17
)

target_link_libraries(network-monitor 
		#"/home/chuongdao/.conan2/p/fmt094320dd109ec/p/lib/libfmt.a"
		# need to link library directly otherwise it wont work properly 
		"${fmt_INCLUDE_DIRS_RELEASE}/../lib/libfmt.a"
		${spdlog_LIBRARIES}
		${Boost_LIBRARIES}		
)
