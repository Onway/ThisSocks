TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    Config.cpp \
    Logger.cpp \
    Passwd.cpp \
    Proxy.cpp \
    TcpServer.cpp \
    Utils.cpp \
    Encrypt.cpp

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
    Proxy.h \
    TcpServer.h \
    Utils.h \
    Encrypt.h

