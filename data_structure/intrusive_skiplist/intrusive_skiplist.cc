#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <ctime>
#include <unistd.h>
#include <memory>
#include <string>
#include <sys/time.h>
#include "gtest/gtest.h"


#define COUNT 10000
#define RAND_LIMIT 100000000

#define TIME_TICK true
#define GET_TIME_OF_DAY(obj, PTR) \
    if (TIME_TICK) gettimeofday(obj, PTR);

#include "alog.h"
#define log_output_level 1
//#define LOG_INFO(arg...) {}
//#define LOG_DEBUG(arg...) {}

template<typename T>
class intrusive_skiplist
{
public:
    class node
    {
    public:
        std::vector<node *> m_forwards{};
        std::vector<node *> m_backwards{};
        bool operator <(const node &rhs)
        {
            return *(T*)this < *(T*)&rhs;
        }
        bool operator ==(const node &rhs)
        {
            return *(T*)this == *(T*)&rhs;
        }
        void set_link(size_t level, node *obj)
        {
            if (level >= this->m_forwards.size()) {
                this->m_forwards.resize(level + 1);
            }
            this->m_forwards[level] = obj;
            if (obj == nullptr) return;
            if (level >= obj->m_backwards.size()){
                obj->m_backwards.resize(level + 1);
            }
            obj->m_backwards[level] = this;
        }
    };

    node* lowerbound(node *node)
    {
        return locate(node);
    }

    int erase(node *node) 
    {
        for (auto i = 0; i < node->m_forwards.size(); i++) {
            auto prev = node->m_backwards[i];
            auto next = node->m_forwards[i];
            prev->set_link(i, next);
        }
        return 0;
    }

    void insert(node *node)
    {
        LOG_DEBUG("node: `", ((T*)node)->val);
        auto p = m_header;
        m_update.resize(m_max_level + 1);
        for (int i = m_max_level; i >= 0; i--){
            while (p->m_forwards[i] != nullptr && *(p->m_forwards[i]) < *node){
                p = p->m_forwards[i];
            } 
            m_update[i] = p;
        }
        int randLevel = 0;
        while (randLevel <= m_max_level && !(rand() % LEVEL_UP_RATIO)) randLevel++;
        if (randLevel > MAX_LEVEL_LIMIT) randLevel = MAX_LEVEL_LIMIT;
        if (randLevel > m_max_level) {
            randLevel = ++m_max_level;
            m_update.push_back(m_header);
            m_header->set_link(m_max_level, nullptr);
            LOG_DEBUG("level increased. (`)", m_max_level);
        }
        for (auto i = randLevel; i >= 0; i--){
            //node->m_forwards[i] = m_update[i]->m_forwards[i];
            node->set_link(i, m_update[i]->m_forwards[i]);
            m_update[i]->set_link(i, node);
        }

    }

    void print()
    {
        LOG_DEBUG("PRINT LIST STRUCTURE");
        for (auto i = m_max_level; i >= 0; i--){
            auto p = m_header;
            std::string info = "h ";
            while (p){
                if (p != m_header) {
                    info += std::to_string(((T*)p)->val) + " ";
                }
                p = p->m_forwards[i];
            }
            info+="t";
            printf("level %d: %s\n", i, info.c_str());
        }
    }

    intrusive_skiplist()
    {
        srand(time(0));
        m_header = new node();
        m_header->set_link(0, nullptr);
    }

    ~intrusive_skiplist()
    {
        delete m_header;
    }

private:
    node *m_header;
    int m_max_level = 0;  
    std::vector<node *> m_update;
    const int MAX_LEVEL_LIMIT = 32; 
    const int LEVEL_UP_RATIO = 2;

    node* locate(node *node)
    {
        auto p = m_header;
        auto ret = m_header;
        for (int i = m_max_level; i >= 0; i--){
            p = ret;
            while (p && (p == m_header || *p < *node)) {
                ret = p;
                p = p->m_forwards[i];
            }
            m_update[i] = ret;
        }
        return p;
    }
};

/* ====================== Unit Test Below ========================= */

class intrusive_object : public intrusive_skiplist<intrusive_object>::node
{
public:
    int val;
    intrusive_object(int val): val(val){};
    bool operator < (const intrusive_object &rhs)
    {
        return val < rhs.val;
    }
    bool operator == (const intrusive_object &rhs)
    {
        return val == rhs.val;
    }
};

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
    int n = 500000, s = n/5;
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
    LOG_INFO("` ` | ` `", t0.tv_sec, t0.tv_usec, t1.tv_sec, t1.tv_usec);
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


int main(int argc, char **argv)
{
    int seed = /* 153900615;// */time(0);
    srand(seed);
	::testing::InitGoogleTest(&argc, argv);	
	auto ret = RUN_ALL_TESTS();
	return ret;
}