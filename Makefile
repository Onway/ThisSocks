SOURCES += main.cpp Config.cpp Encrypt.cpp Logger.cpp TcpServer.cpp Passwd.cpp \
		   Utils.cpp proxy/Proxy.cpp proxy/SocksProxy.cpp proxy/HttpsProxy.cpp \
		   proxy/HttpProxy.cpp
HEADERS += Config.h Encrypt.h Logger.h TcpServer.h Passwd.h Utils.h \
		   proxy/Proxy.h proxy/SocksProxy.h proxy/HttpsProxy.h \
		   proxy/HttpProxy.h

ThisSocks : $(SOURCES) $(HEADERS)
	g++ $(SOURCES) -o $@ -g

clean :
	rm -f ThisSocks *.o proxy/*.o
