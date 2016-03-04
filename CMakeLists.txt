PROJECT(ThisSocks)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE_DIRECTORIES(
	inc
)

SET(SRCS
	src/main.cpp
	src/Utils.cpp
	src/Logger.cpp
	src/Encrypt.cpp
	src/Config.cpp
	src/Passwd.cpp
	src/TcpServer.cpp
	src/Proxy.cpp
	src/SocksProxy.cpp
	src/HttpsProxy.cpp
	src/HttpProxy.cpp
	src/ClientProxy.cpp
)
ADD_EXECUTABLE(ThisSocks ${SRCS})
