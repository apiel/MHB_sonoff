#include "../timer.cpp"
#include <gtest/gtest.h>

#include "../action.h"

class Hello: public Action {
    public:
        virtual void operator() (int key);
		int called = 0;
};

void Hello::operator() (int key)
{
	called++;
}

Hello hello = Hello();

// int hello_called = 0;
// void hello() {
// 	hello_called++;
// }

void clear_timer() {
	for(int pos = 0; pos < TIMER_SIZE; pos++) {
		timer[pos].time = 0;
	}
}

void full_timer() {
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
}

TEST(add_timer, add)
{
	clear_timer();

	unsigned long current_time = sdk_system_get_time();
	ASSERT_EQ(0, add_timer(&hello, 0, 10, 0));
	ASSERT_EQ(current_time + 10 * 1000000, timer[0].time);
	ASSERT_EQ(&hello, timer[0].object);
	ASSERT_EQ(0, timer[0].action);
	ASSERT_EQ(0, timer[0].id);

	ASSERT_EQ(1, add_timer(&hello, 1, 20, 5));
	ASSERT_EQ(current_time + 20 * 1000000, timer[1].time);
	ASSERT_EQ(&hello, timer[1].object);
	ASSERT_EQ(1, timer[1].action);
	ASSERT_EQ(5, timer[1].id);

	ASSERT_EQ(2, add_timer(&hello, 3, 10, 2));
	ASSERT_EQ(current_time + 10 * 1000000, timer[2].time);
	ASSERT_EQ(&hello, timer[2].object);
	ASSERT_EQ(3, timer[2].action);
	ASSERT_EQ(2, timer[2].id);
}

TEST(add_timer, add_full)
{
	clear_timer();
	full_timer();
	ASSERT_EQ(-1, add_timer(&hello, 0, 10, 0));
}

TEST(get_free_timer, get)
{
	clear_timer();
	ASSERT_EQ(0, get_free_timer());
	full_timer();
	ASSERT_EQ(-1, get_free_timer());
	sdk_current_time += 50 * 1000000;
	ASSERT_EQ(0, get_free_timer());
	add_timer(&hello, 0, 10, 0);
	ASSERT_EQ(1, get_free_timer());
}

TEST(execute_timer, execute)
{
	clear_timer();
	hello.called = 0;
	add_timer(&hello, 0, 10, 0);
	execute_timer();
	ASSERT_EQ(0, hello.called);
	sdk_current_time += 50 * 1000000;
	execute_timer();
	ASSERT_EQ(1, hello.called);

	hello.called = 0;
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	add_timer(&hello, 0, 10, 0);
	execute_timer();
	ASSERT_EQ(0, hello.called);
	sdk_current_time += 50 * 1000000;
	execute_timer();
	ASSERT_EQ(3, hello.called);
}
