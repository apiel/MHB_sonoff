#include <gtest/gtest.h>
#include "utils.cpp"

// mocking is just bullshit in cpp
// #include "web.cpp"

// mock https://games.greggman.com/game/using-a-mock-library-to-make-unit-testing-easier-in-c/
// embedded https://www.codeproject.com/Articles/1040972/Using-GoogleTest-and-GoogleMock-frameworks-for-emb

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
