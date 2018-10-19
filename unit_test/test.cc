#include "test.h"

// class ConcurrencyTest : public ::testing::Test
// {
// public:


//     void increment() 
//     {
//         RWLock lock(m_mtx, RWLock::WRITE);
//         value++;
//     }
    
//     int get_value()
//     { 
//         RWLock lock(m_mtx);
//         return value;   
//     }    
    
// private:
//     SharedLock m_mtx;
//     int value = 0;
// };

// TEST_F(ConcurrencyTest, rwlock/* rwlock */)
// {
//     std::mutex mtx;
//     std::vector<std::thread> pool{};
    
//     for (auto i = 0; i < 3; i++){
//         for (int i = 0; i < 10; i++)
//             pool.emplace_back([&]() { increment(); });
//         for (auto j = 0; j < 10; j++){
//             std::thread read([&](){
//                 usleep(100);
//                 std::lock_guard<std::mutex> lck(mtx);
//                 std::cout << get_value() << '\n';
//             });
//             read.detach();
//         }
//         // usleep(10000);           
//     }       
//     for (auto &thd : pool)
//         thd.join();    
//     // {
//     //     std::lock_guard<std::mutex> lck(mtx);
//     //     printf("wait.\n");
//     // }
//     // thread1.join();
//     // thread2.join();
    
// }

template<typename T1, typename T2>
class Pair
{
public:
    Pair(){};
    T1& first() const
    {
        return *(T1*)m_val;
    }
    T2& second() const
    {
        return *(T2*)(m_val+sizeof(T1));
    }
private:
    char m_val[sizeof(T1) + sizeof(T2)];
};

int main(int argc, char **argv)
{
    Pair<int, double> obj;
    // int seed = /* 153900615;// */time(0);
    // srand(seed);
	// ::testing::InitGoogleTest(&argc, argv);	
	// auto ret = RUN_ALL_TESTS();
	//return ret;
    return 0;
}