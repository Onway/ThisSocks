SOURCES += src/main.cpp \
		   src/Config.cpp \
		   src/Encrypt.cpp \
		   src/Logger.cpp \
		   src/TcpServer.cpp \
		   src/Passwd.cpp \
		   src/Utils.cpp \
		   src/Proxy.cpp \
		   src/SocksProxy.cpp \
		   src/HttpsProxy.cpp \
		   src/HttpProxy.cpp \
		   src/ClientProxy.cpp

HEADERS += inc/Config.h \
		   inc/Encrypt.h \
		   inc/Logger.h \
		   inc/TcpServer.h \
		   inc/Passwd.h \
		   inc/Utils.h \
		   inc/Proxy.h \
		   inc/SocksProxy.h \
		   inc/HttpsProxy.h \
		   inc/HttpProxy.h \
		   inc/ClientProxy.h

FLAGS = -I inc/ -g

ThisSocks : $(SOURCES) $(HEADERS)
	g++ $(FLAGS) $(SOURCES) -o $@

clean :
	rm -f ThisSocks *.o proxy/*.o
