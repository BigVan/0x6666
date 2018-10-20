#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <memory>
#include <thread>
#include <queue>
#include <exception>
#include <vector>
#include <gtest/gtest.h>

template<typename T>
class BlockingQueue
{
public:
    BlockingQueue(size_t max_size) : m_max_size(max_size)
    {
        std::cout<<"build blocking queue with size: "<<m_max_size<<std::endl;
    }

    int push(const T &item)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_items.size() == m_max_size) {
            cv_push.wait(lock);
        }
        m_items.push(item);
        cv_pop.notify_one();
        return 0;
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_items.size() == 0) {
            cv_pop.wait(lock);
        }
        auto ret = m_items.front();
        m_items.pop();
        cv_push.notify_one();
        return ret;
    }
   
private:
    std::queue<T> m_items;
    std::mutex m_mtx;
    std::condition_variable cv_push;
    std::condition_variable cv_pop;

    size_t m_max_size = 0;
};

TEST(BlockingQueue, test)
{
    BlockingQueue<int> queue(5);
    std::mutex mtx;

    std::thread thd0([&](){
        for (auto i = 0; i < 100; i++){
            {
                std::unique_lock<std::mutex> lck(mtx);
                std::cout<<"push: "<<i<<std::endl;
            }
            queue.push(i);
           // usleep(200000);
        }
    });

    std::vector<std::thread> pool;
    for (auto i = 0; i < 10; i++){
        std::thread thd( [&](){
            while (true){
                auto ret = queue.pop();
                {
                    std::unique_lock<std::mutex> lck(mtx);
                    std::cout<<"pop: "<<ret<<std::endl;
                }
                sleep(1);
            }
        });
        thd.detach();
    }
    thd0.join();
}

int main(int argc, char **argv)
{
    int seed = 153900615;//time(0);
    srand(seed);
	::testing::InitGoogleTest(&argc, argv);	
	auto ret = RUN_ALL_TESTS();
	return ret;
}
