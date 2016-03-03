PROJECT(ThisSocks)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(SRCS
	main.cpp
	Utils.cpp
	Logger.cpp
	Encrypt.cpp
	Config.cpp
	Passwd.cpp
	TcpServer.cpp
	proxy/Proxy.cpp
	proxy/SocksProxy.cpp
	proxy/HttpsProxy.cpp
	proxy/HttpProxy.cpp
	proxy/ClientProxy.cpp
)
ADD_EXECUTABLE(ThisSocks ${SRCS})
