CPPFLAGS += -isystem include

CXXFLAGS += -g -pthread -lpthread

GTEST_HEADER = include/gtest/*.h include/gtest/internal/*.h

all : TestConfig TestUtils TestPasswd



TestConfig : Config.o ConfigTest.o Utils.o Logger.o gtest_main.a
	g++ $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

Config.o : ../Config.cpp ../Config.h
	g++ -c ../Config.cpp -g

ConfigTest.o : ../Config.h ConfigTest.cpp $(GTEST_HEADER)
	g++ $(CPPFLAGS) $(CXXFLAGS) -c ConfigTest.cpp



TestUtils : Utils.o UtilsTest.o Logger.o gtest_main.a
	g++ $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

Utils.o : ../Utils.h ../Utils.cpp
	g++ -c ../Utils.cpp -g

UtilsTest.o : UtilsTest.cpp $(GTEST_HEADER)
	g++ $(CPPFLAGS) $(CXXFLAGS) -c UtilsTest.cpp



Proxy.o : ../Proxy.cpp ../Proxy.h
	g++ -c ../Proxy.cpp -g


Logger.o : ../Logger.cpp ../Logger.h
	g++ -c ../Logger.cpp -g


TestPasswd : PasswdTest.o Passwd.o Utils.o Logger.o Config.o gtest_main.a
	g++ $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

Passwd.o : ../Passwd.cpp ../Passwd.h
	g++ -c ../Passwd.cpp -g

PasswdTest.o : PasswdTest.cpp $(GTEST_HEADER)
	g++ $(CPPFLAGS) $(CXXFLAGS) -c PasswdTest.cpp



clean :
	rm -f TestConfig TestUtils TestPasswd *.o
