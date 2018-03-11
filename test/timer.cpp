#include "../timer.cpp"
#include <gtest/gtest.h>

int hello_called = 0;
void hello() {
	hello_called++;
}

void clear_timer() {
	for(int pos = 0; pos < TIMER_SIZE; pos++) {
		timer[pos].time = 0;
	}
}

void full_timer() {
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);
}

TEST(add_timer, add)
{
	clear_timer();

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

TEST(add_timer, add_full)
{
	clear_timer();
	full_timer();
	ASSERT_EQ(-1, add_timer(hello, 10));
}

TEST(get_free_timer, get)
{
	clear_timer();
	ASSERT_EQ(0, get_free_timer());
	full_timer();
	ASSERT_EQ(-1, get_free_timer());
	sdk_current_time += 50 * 1000000;
	ASSERT_EQ(0, get_free_timer());
	add_timer(hello, 10);
	ASSERT_EQ(1, get_free_timer());
}

TEST(execute_timer, execute)
{
	clear_timer();
	hello_called = 0;
	add_timer(hello, 10);
	execute_timer();
	ASSERT_EQ(0, hello_called);
	sdk_current_time += 50 * 1000000;
	execute_timer();
	ASSERT_EQ(1, hello_called);

	hello_called = 0;
	add_timer(hello, 10);
	add_timer(hello, 10);
	add_timer(hello, 10);	
	execute_timer();
	ASSERT_EQ(0, hello_called);
	sdk_current_time += 50 * 1000000;
	execute_timer();
	ASSERT_EQ(3, hello_called);
}
