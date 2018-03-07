// #include "../whattotest.cpp"
#include "../utils.cpp"
#include <gtest/gtest.h>

TEST(char_replace_Test, abc)
{
	char str[10] = "a.b.c";
	char * test = str;
	char_replace(test, '.', '-');
	ASSERT_STREQ("a-b-c", test);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
