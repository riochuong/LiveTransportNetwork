cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

set(CMAKE_VERBOSE_MAKEFILE ON)


find_package(Boost 1.81.0 REQUIRED COMPONENTS system)
find_package(GTest 1.13.0 REQUIRED)
find_package(spdlog 1.10.0 REQUIRED)
find_package(fmt 8.1.1 REQUIRED)
find_package(OpenSSL 3.1.0 REQUIRED)
find_package(ZLIB 1.2.13 REQUIRED)



# need to specify include directories here 
include_directories("${GTest_INCLUDE_DIRS}")
include_directories("${spdlog_INCLUDE_DIRS}")
include_directories("${fmt_INCLUDE_DIR}")
include_directories("${Boost_INCLUDE_DIR}")
include_directories("${OpenSSL_INCLUDE_DIR}")


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

target_compile_definitions(websocket-client-test
	PRIVATE
		TEST_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/cacert.pem"

)

target_link_libraries(websocket-client-test
		network-monitor
		"${GTest_INCLUDE_DIRS}/../lib/libgtest.a"
		${spdlog_LIBRARIES}
		"${fmt_INCLUDE_DIRS_RELEASE}/../lib/libfmt.a"
		${Boost_LIBRARIES}
		#ZLIB::ZLIB
		"${OpenSSL_INCLUDE_DIRS}/../lib/libssl.a"
		"${OpenSSL_INCLUDE_DIRS}/../lib/libcrypto.a"
		"/home/chuongdao/.conan2/p/zlib4be8ddd7aa752/p/lib/libz.a"
)

add_test(websocket-client-test websocket-client-test)
