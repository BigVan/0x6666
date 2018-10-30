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
#ifdef __DEBUG
        printf("node: %d\n", ((T*)node)->val);
#endif
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
#ifdef __DEBUG
            printf("level increased. (`)", m_max_level);
#endif
        }
        for (auto i = randLevel; i >= 0; i--){
            //node->m_forwards[i] = m_update[i]->m_forwards[i];
            node->set_link(i, m_update[i]->m_forwards[i]);
            m_update[i]->set_link(i, node);
        }

    }

    void print()
    {
        printf("PRINT LIST STRUCTURE\n");
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
    const int LEVEL_UP_RATIO = 2;
    const int MAX_LEVEL_LIMIT = 32;

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
