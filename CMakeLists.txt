option(BUILD_UNIT_TEST "build unit test" OFF)
option(USE_CRYPTOPP "use libcrypto++" OFF)

project(ThisSocks)
cmake_minimum_required(VERSION 2.8)

include_directories(
	${PROJECT_SOURCE_DIR}/inc
	${PROJECT_SOURCE_DIR}/test/include
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(SRCS
	src/Utils.cpp
	src/Logger.cpp
	src/Encrypt.cpp
	src/Config.cpp
	src/Passwd.cpp
    src/Recorder.cpp
	src/TcpServer.cpp
	src/Proxy.cpp
	src/ServerProxy.cpp
	src/SocksProxy.cpp
	src/HttpsProxy.cpp
	src/HttpProxy.cpp
	src/ClientProxy.cpp
)
add_executable(ThisSocks ${SRCS} src/main.cpp)
target_link_libraries(ThisSocks pthread)
if (USE_CRYPTOPP)
	add_definitions(-DUSE_CRYPTOPP)
	target_link_libraries(ThisSocks cryptopp)
endif()

set(CMAKE_INSTALL_PREFIX /usr/local)
install(PROGRAMS ${PROJECT_BINARY_DIR}/ThisSocks DESTINATION bin)
install(
	PROGRAMS
	script/ThisSocks
	script/ThisSocks_C
	script/ThisSocks_S

	DESTINATION
	/etc/init.d
)
install(
	FILES
	conf/client.conf
	conf/server.conf
	conf/passwd.conf

	DESTINATION
	/etc/ThisSocks
)

if (APPLE)
    set(CP_FILE -f)
    set(CP_DIR -fR)
else()
    set(CP_FILE -u)
    set(CP_DIR -ur)
endif()

execute_process(
	COMMAND cp ${CP_DIR} ${PROJECT_SOURCE_DIR}/conf ${PROJECT_BINARY_DIR}
	COMMAND cp ${CP_FILE} ${PROJECT_SOURCE_DIR}/test/curl.bash ${PROJECT_BINARY_DIR}
)

if (BUILD_UNIT_TEST)
	link_directories(
		${PROJECT_SOURCE_DIR}/test
	)
	set(LIBS
		gtest_main
		gtest
		pthread
	)
	execute_process(
		COMMAND cp ${CP_DIR} ${PROJECT_SOURCE_DIR}/test/test_conf ${PROJECT_BINARY_DIR}
	)

	add_executable(TestConfig ${SRCS} test/ConfigTest.cpp)
	target_link_libraries(TestConfig ${LIBS})

	add_executable(TestPasswd ${SRCS} test/PasswdTest.cpp)
	target_link_libraries(TestPasswd ${LIBS})

	add_executable(TestUtils ${SRCS} test/UtilsTest.cpp)
	target_link_libraries(TestUtils ${LIBS})
endif()
