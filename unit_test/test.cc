#include "test.h"
#include "data_structure/intrusive_skiplist/intrusive_skiplist.h"
#include "data_structure/intrusive_skiplist/concurrent_intrusive_skiplist.h"
#include "../tools/lock.cc"

#define LOG_INFO(arg...) {}
#define LOG_DEBUG(arg...) {}

#define TIME_TICK true
#define GET_TIME_OF_DAY(obj, PTR) \
    if (TIME_TICK) gettimeofday(obj, PTR);

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

class skiplist_test : public ::testing::Test
{
public:
    static const int _LIST_SIZE = 35;
    static const int _RAND_LIMIT = 100;
    static const int _SEARCH_CNT = 20;
    static const int _ERASE_CNT = 15;

    static uint64_t tick(const timeval &t1, const timeval &t0)
    {
        return (t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec); 
    }
};

TEST_F(skiplist_test, test0)
{
    int n = skiplist_test::_LIST_SIZE;
    int randLimit = skiplist_test::_RAND_LIMIT;
    intrusive_skiplist<intrusive_object> list;
    std::vector<intrusive_object *> items{};
    for (int i = 0; i < n; i++){
        auto val = rand() % randLimit;
        items.push_back(new intrusive_object(val));
        list.insert(items.back());
    }
    list.print();
    for (auto i = 0; i < skiplist_test::_SEARCH_CNT; i++){
        auto val = rand() % randLimit;
        auto obj = intrusive_object(val);
        auto ret = static_cast<intrusive_object*>(list.lowerbound(&obj));
        if (ret == nullptr) continue;
        LOG_INFO("lowerbound of `: `", val, ret->val);
        list.print();
    }
    std::random_shuffle(items.begin(), items.end());
    for (auto i = 0; i< skiplist_test::_ERASE_CNT; i++) {
        LOG_INFO("erase: `", items[i]->val);
        list.erase(items[i]);
        list.print();
    }
    LOG_INFO("task finish.");
}

struct cmp{
        bool operator() ( intrusive_object *a,  intrusive_object *b)const{
            return a->val < b->val;
        }
    };


void lb(int s, std::set<intrusive_object *, cmp> &stl, std::vector<intrusive_object *> &data){
    
    for (int i = 0; i < s; i++){
        auto ret = stl.lower_bound(data[i]);
        // if (ret != stl.end())
        //     assert((*ret)->val == ans[i]); 
       // LOG_INFO("` `", ans[i],  (*ret)->val );
    }
}


TEST_F(skiplist_test, vs_stl_set)
{
    std::vector<intrusive_object *> data;
   
    std::set<intrusive_object *, cmp> stl;
    intrusive_skiplist<intrusive_object> list;
    int n = 1<<16, s = n/5;
    int randLimit = 100000000;
    for (auto i = 0; i < n; i++){
        auto val = rand() % randLimit;
        data.push_back(new intrusive_object(val));
    }
    auto search = data;
    std::random_shuffle(search.begin(), search.end());
    search.erase(search.begin() + s, search.end());
    struct timeval t0, t1;
    
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : data){
        list.insert( it );
    }
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_list_insert = skiplist_test::tick(t1, t0);
    
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : data){
        stl.insert( it );
       //stl.insert(it->val);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_set_insert = skiplist_test::tick(t1, t0);

    std::vector<int> ans(s);
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < s; i++){
        auto ret = static_cast<intrusive_object*>(list.lowerbound(search[i]));
        if (ret) ans[i] = ret->val;
        else ans[i] = -1;
    }
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_list_lb = skiplist_test::tick(t1, t0); 
    
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < s; i++){
        /* auto ret = */ stl.lower_bound(search[i]);
        // if (ret != stl.end())
        //     assert((*ret)->val == ans[i]); 
       // LOG_INFO("` `", ans[i],  (*ret)->val );
    }
    //lb(s, stl, data);
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_set_lb = skiplist_test::tick(t1, t0);

    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < s; i++){
        list.erase(search[i]);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_list_erase = skiplist_test::tick(t1, t0); 

    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < s; i++){
       auto ret = stl.lower_bound(search[i]);
        if (ret != stl.end() && (*ret)->val == search[i]->val) { stl.erase(ret); }
    }
    GET_TIME_OF_DAY(&t1, NULL);
    auto c_stl_erase = skiplist_test::tick(t1, t0);

    printf("intrusive_skiplist vs. STL-set\n"\
            "    %d\tinsert(us):     %llu\t%llu\t%.2lf\n"\
            "    %d\tlowerbound(us): %llu\t%llu\t%.2lf\n"\
            "    %d\terase(us):      %llu\t%llu\t%.2lf\n",
            n, c_list_insert, c_set_insert, (double)c_set_insert/c_list_insert, 
            s, c_list_lb, c_set_lb, (double)c_set_lb/c_list_lb,
            s, c_list_erase, c_stl_erase, (double)c_stl_erase/c_list_erase
    );
}

TEST_F(skiplist_test, concurrent)
{
    std::vector<concurrent_intrusive_object *> data1;
    concurrent_intrusive_skiplist<concurrent_intrusive_object> list1;
    int n = 1 << ((rand() % 8 + 10));
    int thread_count = 1<<(rand() % 6 + 1);
    int randLimit = 10000000;
    for (auto i = 0; i < n; i++){
        auto val = rand() % randLimit;
        data1.push_back(new concurrent_intrusive_object(val));
    }
    std::vector<std::thread> pool;
    int cnt = n / thread_count;
    struct timeval t0, t1;
    printf("insert %d items into concurrent-skiplist.\n", n);
    printf("launch %d threads (%d items per thd)\n", thread_count, cnt);
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < thread_count; i++){
        pool.emplace_back([&, i](){
            for (auto j = i * cnt; j < (i + 1) * cnt; j++){
                list1.insert(data1[j]);
            }
        });
    }
    for (auto &thd : pool) thd.join();
    GET_TIME_OF_DAY(&t1, NULL);
    auto c1 = skiplist_test::tick(t1, t0);
    printf("done.(%llu ms)\n", c1/1000);
}

TEST_F(skiplist_test, concurrent_perf)
{
    std::vector<intrusive_object *> data0;
    std::vector<concurrent_intrusive_object *> data1;
    intrusive_skiplist<intrusive_object> list0;
    concurrent_intrusive_skiplist<concurrent_intrusive_object> list1, list2;
    //int n = 500000, s = n/5;
    int n = 1<<20;
    int thread_count = 8;
    int randLimit = 100000000;
    for (auto i = 0; i < n; i++){
        auto val = rand() % randLimit;
        data0.push_back(new intrusive_object(val));
        data1.push_back(new concurrent_intrusive_object(val));
    }
    printf("insert %d items into skiplist.\n", n);
    struct timeval t0, t1;
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < n; i++)
        list0.insert(data0[i]);
    GET_TIME_OF_DAY(&t1, NULL);
    auto c0 = skiplist_test::tick(t1, t0);
    printf("done. (%llu ms)\n", c0/1000);

    std::vector<std::thread> pool;
    int cnt = n / thread_count;
    printf("insert %d items into concurrent-skiplist.\n", n);
    printf("launch %d threads (%d items per thd)\n", thread_count, cnt);
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < thread_count; i++){
        pool.emplace_back([&, i](){
            for (auto j = i * cnt; j < (i + 1) * cnt; j++){
                list1.insert(data1[j]);
            }
        });
    }
    for (auto &thd : pool) thd.join();
    GET_TIME_OF_DAY(&t1, NULL);
    auto c1 = skiplist_test::tick(t1, t0);
    printf("done.(%llu ms)\n", c1/1000);
    printf("Speedup ratio = %llu / %llu = %.2lf\n", c0, c1, (double)c0/c1);
    GET_TIME_OF_DAY(&t0, NULL);
    for (int i = 0; i < n; i++){
        list2.insert(data1[i]);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    c1 = skiplist_test::tick(t1, t0);
    printf("done.(%llu ms)\n", c1/1000);
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