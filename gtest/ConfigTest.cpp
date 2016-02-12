#include "../Config.h"
#include "gtest/gtest.h"

int GetArgc(const char *argv[])
{
	int argc = 0;
	while (argv[argc] != NULL) {
		++argc;
	}
	return argc;
}

TEST(Constructor, Default) {
	const char *argv[] = {
		"Test",
		NULL,
	};

	Config cf;
	cf.Init(GetArgc(argv), const_cast<char **>(argv));
	EXPECT_FALSE(cf.RunAsClient);
	EXPECT_EQ(0, cf.ServerPort);
	EXPECT_EQ("", cf.ServerAddress);
	EXPECT_TRUE(cf.LocalAddress.empty());
	EXPECT_EQ(0, cf.LocalPort);
	EXPECT_TRUE(cf.CfgFile.empty());
}
