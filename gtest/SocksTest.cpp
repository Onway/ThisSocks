#include "../Socks.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace std;

TEST(ReturnValue, Init) {
	Socks socks;
	EXPECT_TRUE(socks.Init("127.0.0.1", 1080));
}
