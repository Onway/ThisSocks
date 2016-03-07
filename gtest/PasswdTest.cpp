#include "Passwd.h"
#include "Logger.h"
#include "Config.h"
#include "gtest/gtest.h"
#include <iostream>

TEST(Passwd, All) {
	Passwd pwd;
	pwd.LoadFile("test_conf/passwd.conf");
	EXPECT_TRUE(pwd.IsValidUser("xiaoming", "123456"));
	EXPECT_FALSE(pwd.IsValidUser("xiaobai", "123456"));
	EXPECT_FALSE(pwd.IsValidUser("xiaohei", "123456"));
}
