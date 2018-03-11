#include "../timer.cpp"
#include <gtest/gtest.h>

void hello() {

}

TEST(add_timer, add)
{
	unsigned long current_time = sdk_system_get_time();
	ASSERT_EQ(0, add_timer(hello, 10));
	ASSERT_EQ(current_time + 10 * 1000000, timer[0].time);
	ASSERT_EQ(hello, timer[0].callback);

	ASSERT_EQ(1, add_timer(hello, 20));
	ASSERT_EQ(current_time + 20 * 1000000, timer[1].time);
	ASSERT_EQ(hello, timer[1].callback);

	ASSERT_EQ(2, add_timer(hello, 10));
	ASSERT_EQ(current_time + 10 * 1000000, timer[2].time);
	ASSERT_EQ(hello, timer[2].callback);
}

TEST(get_free_timer, get)
{
	ASSERT_EQ(3, get_free_timer());
	add_timer(hello, 10);
	ASSERT_EQ(4, get_free_timer());
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	ASSERT_EQ(-1, get_free_timer());
}
