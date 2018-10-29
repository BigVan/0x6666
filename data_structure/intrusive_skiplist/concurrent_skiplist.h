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
#include "../../tools/lock.h"
#include <random>

template<typename T>
class concurrent_intrusive_skiplist
{
public:
    static const int MAX_LEVEL_LIMIT = 32;

    class node
    {
    public:
        ILock *m_shared_lock = nullptr;
        std::vector<node *> m_forwards;//{};
        // std::vector<node *> m_backwards{};
        node(): m_shared_lock(create_shared_lock())
        {
            m_forwards.resize(concurrent_intrusive_skiplist::MAX_LEVEL_LIMIT);
        }
        __attribute__((always_inline))
        bool operator <(const node &rhs)
        { 
            // Lock lck(m_shared_lock, Lock::READ);
            return *(T*)this < *(T*)&rhs;
        }
        __attribute__((always_inline))
        bool operator ==(const node &rhs)
        {
            // Lock lck(m_shared_lock, Lock::READ);
            return *(T*)this == *(T*)&rhs;
        }
        __attribute__((always_inline))
        void set_link(size_t level, node *obj)
        {
            Lock lock(m_shared_lock);
            // if (level >= this->m_forwards.size()) {
            //     this->m_forwards.resize(level + 1); //并发时触发resize可能会导致访存异常
            // }
            this->m_forwards[level] = obj;
        }
        __attribute__((always_inline))
        node* get_forwards(int i)
        {
       //     Lock lck(m_shared_lock, Lock::READ);
            return m_forwards[i];
        }
        ~node()
        {
            delete m_shared_lock;
        }
    };

    node* lowerbound(node *node)
    {
        return locate(node);
    }

    int erase(node *node)
    {
        // for (auto i = 0; i < node->m_forwards.size(); i++) {
        //     // auto prev = node->m_backwards[i];
        //     auto next = node->m_forwards[i];
        //     prev->set_link(i, next);
        // }
        return 0;
    }

    void insert(node *item)
    {
        auto max_level = 0;
        std::vector<node *> update;
        node *p = nullptr;
        update.resize(MAX_LEVEL_LIMIT);
        {
            // Lock shared(m_lock, Lock::READ);
            p = m_header;
            max_level = m_max_level.load();
        }
        for (int i = max_level; i >= 0; i--){
            node *next = nullptr;
            bool ok = false;
            {
                Lock lock(p->m_shared_lock, Lock::READ);
                next = p->m_forwards[i];// p->get_forwards(i);
                ok = (next != nullptr);
            }
            while ( ok /* next != nullptr */){
                Lock lock(next->m_shared_lock, Lock::READ);
                if (!(*next < *item)) {
                    break;
                }
                p = next;
                next =/*  p->get_forwards(i); */ p->m_forwards[i];
                ok = (next != nullptr);
            }
            update[i] = p;
        }
        int randLevel = 0;
        auto rand = [=](){
            static thread_local std::mt19937 generator;
            std::uniform_int_distribution<int> distribution(0, 1024);
            return distribution(generator) % LEVEL_UP_RATIO;
        };
        while (randLevel <= max_level && !rand()) randLevel++;
        if (randLevel > MAX_LEVEL_LIMIT) {
            randLevel = MAX_LEVEL_LIMIT;
        }
        if (randLevel > max_level) {
            
            if (randLevel > m_max_level.load()){
                max_level = m_max_level.fetch_add(1) + 1;
                m_header->set_link(max_level, nullptr);
            }
            update[randLevel] = m_header;
        }
        for (auto i = randLevel; i >= 0; i--){
            {
                Lock shared(update[i]->m_shared_lock, Lock::READ);
                item->set_link(i, update[i]->/* get_forwards(i) */m_forwards[i]);
            }
            update[i]->set_link(i, item);
        }
    }

    void print()
    {
        printf("PRINT LIST STRUCTURE\n");
        for (auto i = m_max_level.load(); i >= 0; i--){
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

    const std::vector<node *> dump()
    {
        std::vector<node *> ret{};
        auto p = m_header;
        while (p != nullptr) {
            ret.push_back(p);
            p = p->m_forwards[0];
        }
        return ret;
    }

    concurrent_intrusive_skiplist()
    {
        m_lock = create_shared_lock();
        srand(time(0));
        m_header = new node();
        m_header->set_link(0, nullptr);
        m_max_level.store(0);
    }

    ~concurrent_intrusive_skiplist()
    {
        delete m_header;
    }

private:
    ILock *m_lock ;//= nullptr;
    node *m_header;
    std::atomic_int8_t m_max_level;
    const int LEVEL_UP_RATIO = 2;

    node* locate(node *node/* , vector<node *> *update */)
    {
        auto p = m_header;
        auto ret = m_header;
        for (int i = m_max_level.load(); i >= 0; i--){
            bool ok = false;
            {
                Lock lock(ret->m_shared_lock, Lock::READ);
                p = ret;
                ok = (p != nullptr || p == m_header);
            }
            while (ok) {
                Lock lock(p->m_shared_lock, Lock::READ);
                if  (!(*p < *node)) break;
                ret = p;
                p =p->m_forwards[i];
                ok = (p != nullptr || p == m_header);
            }
            // if (update) update[i] = ret;
        }
        return p;
    }
};

class concurrent_intrusive_object : 
        public concurrent_intrusive_skiplist<concurrent_intrusive_object>::node
{
public:
    int val;
    concurrent_intrusive_object(int val): val(val){};
    bool operator < (const concurrent_intrusive_object &rhs)
    {
        return val < rhs.val;
    }
    bool operator == (const concurrent_intrusive_object &rhs)
    {
        return val == rhs.val;
    }
};


/* ====================== Unit Test Below ========================= */
