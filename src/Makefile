INC_DIR = ../inc
LIB_DIR = 

STATIC_LIB = 
DYNAMIC_LIB = pthread

CPPFLAGS = 
CXXFLAGS = -Wall $(INC_DIR:%=-I%)
LDFLAGS = -Wl, -Bstatic $(STATIC_LIB:%=-l%) -Wl, -Bdynamic $(DYNAMIC_LIB:%=-l%)

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
	DYNAMIC_LIB += cryptopp
endif

.PHONY : all install uninstall clean

all : $(TARGET)

debug: CXXFLAGS += -g
debug: $(TARGET)

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

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPENDS)
