cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

set(CMAKE_VERBOSE_MAKEFILE ON)


find_package(Boost 1.81.0 REQUIRED COMPONENTS system)
find_package(GTest 1.13.0 REQUIRED)
find_package(spdlog 1.10.0 REQUIRED)
find_package(fmt 8.1.1 REQUIRED)



# need to specify include directories here 
include_directories("${GTest_INCLUDE_DIR}")
include_directories("${spdlog_INCLUDE_DIRS}")
include_directories("${fmt_INCLUDE_DIR}")
include_directories("${Boost_INCLUDE_DIR}")


set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/websocket-client-test.cpp")

add_executable(websocket-client-test ${SOURCES})

target_include_directories(websocket-client-test
    PRIVATE
    ../inc
)

target_compile_features(websocket-client-test
	PRIVATE
	cxx_std_17
)

target_link_libraries(websocket-client-test
		network-monitor
		${GTest_LIBRARIES}
		${spdlog_LIBRARIES}
		${fmt_LIBRARIES}
		${Boost_LIBRARIES}	
)

add_test(websocket-client-test websocket-client-test)
