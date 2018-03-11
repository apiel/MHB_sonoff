#include "../utils.cpp"
#include <gtest/gtest.h>

TEST(char_replace, abc)
{
	char str[10] = "a.b.c";
	char * test = str;
	char_replace(test, '.', '-');
	ASSERT_STREQ("a-b-c", test);
}

TEST(char_to_int, zero)
{
	ASSERT_EQ(0, char_to_int((char *)""));
	ASSERT_EQ(0, char_to_int((char *)"abcd"));
	ASSERT_EQ(0, char_to_int((char *)"ab123cd"));
	ASSERT_EQ(0, char_to_int((char *)"0ab"));
}

TEST(char_to_int, 8123)
{
	ASSERT_EQ(8123, char_to_int((char *)"8123"));
	ASSERT_EQ(8123, char_to_int((char *)"8123abcd"));
}
