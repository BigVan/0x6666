#include "heaptest.h"

int main(int argc, char **argv)
{
    int seed = 153900615;//time(0);
    srand(seed);
	::testing::InitGoogleTest(&argc, argv);	
	auto ret = RUN_ALL_TESTS();
	return ret;
}