TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    Config.cpp \
    Logger.cpp \
    Passwd.cpp \
    TcpServer.cpp \
    Utils.cpp \
    Encrypt.cpp \
    proxy/SocksProxy.cpp \
    proxy/HttpsProxy.cpp \
    proxy/HttpProxy.cpp \
    proxy/Proxy.cpp

OTHER_FILES += \
    client.conf \
    passwd.conf \
    server.conf \
    todolist \
    Makefile

HEADERS += \
    Config.h \
    Logger.h \
    Passwd.h \
    TcpServer.h \
    Utils.h \
    Encrypt.h \
    proxy/SocksProxy.h \
    proxy/HttpsProxy.h \
    proxy/HttpProxy.h \
    proxy/Proxy.h

