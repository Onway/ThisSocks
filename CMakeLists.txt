option(build_unit_test "build unit test" OFF)

project(ThisSocks)
cmake_minimum_required(VERSION 2.8)

include_directories(
	${PROJECT_SOURCE_DIR}/inc
	${PROJECT_SOURCE_DIR}/gtest/include
)

set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_CXX_FLAGS_DEBUG -g)
set(SRCS
	src/Utils.cpp
	src/Logger.cpp
	src/Encrypt.cpp
	src/Config.cpp
	src/Passwd.cpp
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

execute_process(
	COMMAND cp -ur ${PROJECT_SOURCE_DIR}/conf ${PROJECT_BINARY_DIR}
	COMMAND cp -u ${PROJECT_SOURCE_DIR}/gtest/curl.bash ${PROJECT_BINARY_DIR}
)

if (build_unit_test)
	link_directories(
		${PROJECT_SOURCE_DIR}/gtest
	)
	set(LIBS
		gtest_main
		gtest
		pthread
	)
	execute_process(
		COMMAND cp -ur ${PROJECT_SOURCE_DIR}/gtest/test_conf ${PROJECT_BINARY_DIR}
	)

	add_executable(TestConfig ${SRCS} gtest/ConfigTest.cpp)
	target_link_libraries(TestConfig ${LIBS})

	add_executable(TestPasswd ${SRCS} gtest/PasswdTest.cpp)
	target_link_libraries(TestPasswd ${LIBS})

	add_executable(TestUtils ${SRCS} gtest/UtilsTest.cpp)
	target_link_libraries(TestUtils ${LIBS})
endif()
