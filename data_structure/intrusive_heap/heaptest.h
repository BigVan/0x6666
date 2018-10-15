#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <ctime>
#include <unistd.h>
#include <memory>
#include <vector>
#include <list>
#include <algorithm>
#include <utility>
#include <string>
#include <set>
#include <sys/time.h>

#include "gtest/gtest.h"

#define COUNT 10000
#define RAND_LIMIT 100000000

#define TIME_TICK true
#define GET_TIME_OF_DAY(obj, PTR) \
    if (TIME_TICK) gettimeofday(obj, PTR);



class Heap : public  ::testing::Test
{
public:
	
	const static int PUSH = 0;
    const static int POP = 1;
    const static int ERASE = 2;

	std::vector<std::pair<int, int>> pairs;
	std::vector<std::pair<int, int>> makedata(int op_cnt, int rand_limit, int op_type)
	{
		std::vector<std::pair<int, int>> ret{};
		for (int i = 0; i < op_cnt; i++) {
			int op = rand() % op_type;
			if (op == Heap::POP) {
				ret.push_back(std::make_pair(op, -1));
				continue;
			}
			auto val = rand() % rand_limit;
			ret.push_back(std::make_pair(op, val));
    	}
		return ret;
	}

    virtual void SetUp() override
    {
        pairs = makedata(COUNT, RAND_LIMIT, 1);
    }

};



