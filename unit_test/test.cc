#include "test.h"
#include "../tools/lock.cc"

class LockTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        int size = 10000;
        int rand_limit = 1000000;
        for (int i = 0; i < size; i++){
            data.push_back(rand() % rand_limit);
            val.push_back(rand() % rand_limit);
        }
    }

    void lowerbound()
    {
        int writers = 30;
        int readers = 100;
        std::set<uint64_t> items{};
        std::vector<std::thread> pool;
        for (int i = 0; i < writers; i++){
            pool.emplace_back([&](){
                for (auto it : data){
                    Lock lock(m_mtx);
                    items.insert(it);
                }
            });
        }
        for (auto i = 0; i < readers; i++) {
            pool.emplace_back( [&](){
                for (auto it : val) {
                    Lock lock(m_mtx, Lock::READ);    
                    auto ret = items.lower_bound(it);
                }
            });
        }
        for (auto &thd : pool) thd.join();
    }

    ILock *m_mtx = nullptr;
    std::vector<uint64_t> data{};
    std::vector<uint64_t> val{};

};

TEST_F(LockTest, shared_lock_set)
{
    m_mtx = create_shared_lock();
    lowerbound();
    delete m_mtx;
}

TEST_F(LockTest, mutex_lock_set)
{
    m_mtx = create_mutex_lock();
    lowerbound();
    delete m_mtx;
}


int main(int argc, char **argv)
{
    int seed = 153900615;//time(0);
    srand(seed);
	::testing::InitGoogleTest(&argc, argv);	
	auto ret = RUN_ALL_TESTS();
	return ret;
    return 0;
}