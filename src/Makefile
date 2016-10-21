INC_DIR = 

CPPFLAGS = 
CXXFLAGS = -Wall $(INC_DIR:%=-I%)
LDFLAGS = -lpthread

TARGET = ThisSocks
SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:%.cc=%.o)
DEPENDS = $(SOURCES:%.cc=%.d)

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

$(DEPENDS) : %.d : %.cc
	rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJECTS) : %.o : %.cc
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
