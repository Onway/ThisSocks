INC_DIR = ../inc

CPPFLAGS = 
CXXFLAGS = -Wall $(INC_DIR:%=-I%)
LDFLAGS = -lpthread

TARGET = ThisSocks
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
DEPENDS = $(SOURCES:%.cpp=%.d)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
endif

USE_CRYPTOPP ?= 0
ifeq ($(USE_CRYPTOPP), 1)
	CPPFLAGS += -DUSE_CRYPTOPP
	LDFLAGS += -lcryptopp
endif

.PHONY : all install uninstall clean

all : $(TARGET)

$(DEPENDS) : %.d : %.cpp
	rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJECTS) : %.o : %.cpp
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)

$(TARGET) : $(DEPENDS) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPENDS)
endif

OS = $(shell uname -s)
install:
	mkdir -p /etc/$(TARGET)
	cp -f ../conf/* /etc/$(TARGET)
	cp $(TARGET) /usr/local/bin
	if test "$(OS)" != "Darwin"; then \
		cp -f ../script/* /etc/init.d; \
	fi;

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -rf /etc/$(TARGET)
	if test "$(OS)" != "Darwin"; then \
		rm -rf /etc/init.d/$(TARGET)*; \
	fi;

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPENDS)
