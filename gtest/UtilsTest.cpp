#include "../Utils.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace std;

TEST(TrimStart, Default) {
	string str;

	str = "";
	ASSERT_FALSE(Utils::Utils::TrimStart(str));
	ASSERT_EQ("", str);

	str = " ";
	ASSERT_TRUE(Utils::TrimStart(str));
	ASSERT_EQ("", str);
	
	str = "\t";
	ASSERT_TRUE(Utils::TrimStart(str));
	ASSERT_EQ("", str);
	
	str = "\t ";
	ASSERT_TRUE(Utils::TrimStart(str));
	ASSERT_EQ("", str);

	str = "abc";
	ASSERT_FALSE(Utils::TrimStart(str));
	ASSERT_EQ("abc", str);

	str = " \tabc";
	ASSERT_TRUE(Utils::TrimStart(str));
	ASSERT_EQ("abc", str);
	
	str = "       abc ";
	ASSERT_TRUE(Utils::TrimStart(str));
	ASSERT_EQ("abc ", str);
}

TEST(TrimEnd, Default) {
	string str;

	str = "";
	ASSERT_FALSE(Utils::TrimEnd(str));
	ASSERT_EQ("", str);

	str = " ";
	ASSERT_TRUE(Utils::TrimEnd(str));
	ASSERT_EQ("", str);
	
	str = "\t";
	ASSERT_TRUE(Utils::TrimEnd(str));
	ASSERT_EQ("", str);
	
	str = "\t ";
	ASSERT_TRUE(Utils::TrimEnd(str));
	ASSERT_EQ("", str);

	str = "abc";
	ASSERT_FALSE(Utils::TrimEnd(str));
	ASSERT_EQ("abc", str);

	str = "abc \t";
	ASSERT_TRUE(Utils::TrimEnd(str));
	ASSERT_EQ("abc", str);
	
	str = " abc    ";
	ASSERT_TRUE(Utils::TrimEnd(str));
	ASSERT_EQ(" abc", str);
}

TEST(Trim, Default) {
	string str;

	str = "abc";
	ASSERT_FALSE(Utils::Trim(str));
	ASSERT_EQ("abc", str);

	str = " abc";
	ASSERT_TRUE(Utils::Trim(str));
	ASSERT_EQ("abc", str);

	str = "abc ";
	ASSERT_TRUE(Utils::Trim(str));
	ASSERT_EQ("abc", str);

	str = " abc ";
	ASSERT_TRUE(Utils::Trim(str));
	ASSERT_EQ("abc", str);
}

TEST(Split, Default) {
	vector<string> result;

	string str = "";
	Utils::Split(str, '=', result);
	EXPECT_TRUE(result.empty());

	str = "=";
	Utils::Split(str, '=', result);
	ASSERT_EQ(2, result.size());
	EXPECT_EQ("", result[0]);
	EXPECT_EQ("", result[1]);

	str = " =";
	Utils::Split(str, '=', result);
	ASSERT_EQ(2, result.size());
	EXPECT_EQ(" ", result[0]);
	EXPECT_EQ("", result[1]);
	
	str = "= ";
	Utils::Split(str, '=', result);
	ASSERT_EQ(2, result.size());
	EXPECT_EQ("", result[0]);
	EXPECT_EQ(" ", result[1]);
	
	str = " = ";
	Utils::Split(str, '=', result);
	ASSERT_EQ(2, result.size());
	EXPECT_EQ(" ", result[0]);
	EXPECT_EQ(" ", result[1]);

	str = "==";
	Utils::Split(str, '=', result);
	ASSERT_EQ(3, result.size());
	EXPECT_EQ("", result[0]);
	EXPECT_EQ("", result[1]);
	EXPECT_EQ("", result[2]);

	str = "= =";
	Utils::Split(str, '=', result);
	ASSERT_EQ(3, result.size());
	EXPECT_EQ("", result[0]);
	EXPECT_EQ(" ", result[1]);
	EXPECT_EQ("", result[2]);

	str = "abc";
	Utils::Split(str, '=', result);
	ASSERT_EQ(1, result.size());
	EXPECT_EQ("abc", result[0]);
}
