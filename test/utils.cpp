#include "../utils.cpp"
#include <gtest/gtest.h>

TEST(char_replace_Test, abc)
{
	char str[10] = "a.b.c";
	char * test = str;
	char_replace(test, '.', '-');
	ASSERT_STREQ("a-b-c", test);
}
