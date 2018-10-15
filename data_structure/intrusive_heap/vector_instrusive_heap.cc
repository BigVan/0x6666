
#include "heaptest.h"

class intrusive_object;
struct heap_op {
	int op;
	intrusive_object *v;
};

template<typename T>
class intrusive_heap
{
public:
    class node
    {
    public:
        int idx; 
        // left = idx * 2 + 1; right = (idx + 1) * 2; parent = (idx - 1) / 2;
        bool operator < (const node &rhs) const
        {
            return ((*(T*)this) < (*(T*)&rhs));
        }
    };
    
    void push(node *obj)
    {
        m_nodes.push_back(obj);
        obj->idx = m_nodes.size() - 1;
        up(obj->idx);
    }

    node* pop()
    {
        swap(0, m_nodes.size() - 1);
        auto ret = m_nodes.back();
        m_nodes.pop_back();
        down(0);
        return ret;
    }

    void print()
    {
        for (auto it : m_nodes){
            print_info(it);
        }
    }

    void erase(node *obj)
    {
        auto id = obj->idx;
        swap(obj->idx, m_nodes.size() - 1);
        m_nodes.pop_back();
        if (obj->idx == m_nodes.size()) return;
        auto tmp = m_nodes[id];
        up(tmp->idx);
        down(tmp->idx);
    }

    void reserve(int size){ m_nodes.reserve(size); }

    size_t size(){ return m_nodes.size(); }

    bool empty() { return m_nodes.size() == 0; }

private:
    std::vector<node *> m_nodes;

    void swap(int i, int j)
    {
        auto tmp = m_nodes[i];
        m_nodes[i] = m_nodes[j];
        m_nodes[j] = tmp;
        m_nodes[i]->idx = i;
        m_nodes[j]->idx = j;
    }

    void up(int idx)
    {
        while (idx != 0){
            auto cmpIdx = (idx - 1) >> 1;
            if (*m_nodes[idx] < *m_nodes[cmpIdx]){
                swap(idx, cmpIdx);
                idx = cmpIdx;
                continue;
            } 
            break;
        }
    }

    void down(int idx)
    {
        int cmpIdx = (idx << 1) + 1;
        while (cmpIdx < m_nodes.size())
        {
            if (cmpIdx + 1 < m_nodes.size() && (*m_nodes[cmpIdx + 1]) < (*m_nodes[cmpIdx])){
                cmpIdx++;
            }
            if (*m_nodes[cmpIdx] < *m_nodes[idx]){
                swap(idx, cmpIdx);
                idx = cmpIdx;
                cmpIdx = (idx << 1) + 1;
                continue;
            } 
            break;
        }
    }
};

class intrusive_object : public intrusive_heap<intrusive_object>::node
{
public:
    int val;
    bool operator <(const intrusive_object &obj)
    {
        return val < obj.val;
    }
    intrusive_object(int v) : val(v) {};
};

TEST_F(Heap, vectorBaseErase)
{
    int count = COUNT;
    int rand_limit = RAND_LIMIT;
    std::vector<heap_op> pushs, pops;
    auto make_test = [&](std::vector<heap_op> &pushs, std::vector<heap_op> &pops)
    {
        for (auto pair : this->pairs){
            if ( pair.first == Heap::POP) {
                pops.push_back(heap_op{ 1 });
                continue;
            }
            heap_op op;
            op.op = Heap::PUSH;
            op.v = new intrusive_object(pair.second);
            pushs.push_back(op/* heap_op{ Heap::PUSH, new intrusive_object(pair.second) } */);
        }
        return;
    };
    make_test(pushs, pops);

    struct timeval t0, t1;
    uint64_t c_push = 0, c_erase = 0;
    uint64_t push_times = pushs.size();
    auto tick = [](const timeval &t2, const timeval &t1){
        return (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    };
    
    intrusive_heap<intrusive_object> heap;
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : pushs){
        heap.push(it.v);
    }

    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);
    uint64_t erase_times = heap.size() / 10;
    std::vector<heap_op> erases(pushs.size());
    std::copy(pushs.begin(), pushs.end(), erases.begin());
    for (int i = 0; i<erase_times; i++){
        int k = rand() % pushs.size();
        auto it = erases[i];
        erases[i] = erases[k];
        erases[k] = it;
    }
    erases.erase(erases.begin() + erase_times, erases.end());
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : erases){
        heap.erase(it.v);
    }

    GET_TIME_OF_DAY(&t1, NULL);
    if (TIME_TICK) {
        c_erase = tick(t1, t0);
        printf("Intrusive Heap. time cost: %llu ms\n"\
            "    push: %llu op / %llu us = %llu opus\n"\
            "    erase : %llu op / %llu us = %llu opus\n"\
            "    average  : %llu op / %llu us = %llu opus\n",
            (c_erase + c_push) / 1000,
            push_times, c_push, push_times / c_push,
            erase_times,  c_erase, erase_times / c_erase,
            push_times + erase_times, c_push + c_erase, (push_times + erase_times)/(c_push + c_erase)
            );
    }
    std::set<intrusive_object *> set;
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : pushs){
        set.insert(it.v);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : erases){
        set.erase(it.v);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    if (TIME_TICK) {
        c_erase = tick(t1, t0);
        printf("STL set. time cost: %llu ms\n"\
            "    push: %llu op / %llu us = %llu opus\n"\
            "    erase : %llu op / %llu us = %llu opus\n"\
            "    average  : %llu op / %llu us = %llu opus\n",
            (c_erase + c_push) / 1000,
            push_times, c_push, push_times / c_push,
            erase_times,  c_erase, erase_times / c_erase,
            push_times + erase_times, c_push + c_erase, (push_times + erase_times)/(c_push + c_erase)
            );
    }
}

TEST_F(Heap, vectorBaseSort)
{
    int count = COUNT, rand_limit = RAND_LIMIT;
    std::vector<heap_op> pushs, pops;
    std::vector<intrusive_object *> arr;
    auto cmp =  [](intrusive_object *a, intrusive_object *b){
        return a->val >= b->val;
    } ;
    auto make_test = [&](std::vector<heap_op> &pushs, std::vector<heap_op> &pops)
    {
        for (auto pair : this->pairs){
            if ( pair.first == Heap::POP) {
                pops.push_back(heap_op{ 1 });
                continue;
            }
            pushs.push_back(heap_op{ Heap::PUSH, new intrusive_object(pair.second) });
        }
        return;
    };

    std::make_heap(arr.begin(), arr.end(), cmp);
    make_test(pushs, pops);
    intrusive_heap<intrusive_object> heap;
    uint64_t c_push = 0, c_pop = 0;
    uint64_t push_times = pushs.size(), pop_times = pushs.size();
    auto tick = [](const timeval &t2, const timeval &t1){
        return (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    };
    struct timeval t0, t1;
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : pushs){
        heap.push(it.v);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);
    std::vector<int> judge;
    GET_TIME_OF_DAY(&t0, NULL);
    while (!heap.empty()){
        auto ret = static_cast<intrusive_object *>(heap.pop());
        if (!TIME_TICK){
            judge.push_back(ret->val);
        }
    }
    GET_TIME_OF_DAY(&t1, NULL);
    if (TIME_TICK) {
        c_pop = tick(t1, t0);
        printf("Intrusive Heap (Base on vector). time cost: %llu ms\n"\
            "    push: %llu op / %llu us = %llu opus\n"\
            "    pop : %llu op / %llu us = %llu opus\n"\
            "    average  : %llu op / %llu us = %llu opus\n",
            (c_pop + c_push) / 1000,
            push_times, c_push, push_times / c_push,
            pop_times,  c_pop, pop_times / c_pop,
            push_times + pop_times, c_push + c_pop, (push_times + pop_times)/(c_push + c_pop)
        );
    }
    GET_TIME_OF_DAY(&t0, NULL);
    for (auto it : pushs){
        arr.push_back(it.v);
        std::push_heap(arr.begin(), arr.end(), cmp);
    }
    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);

    auto iter = judge.begin();
    GET_TIME_OF_DAY(&t0, NULL);
    while (!arr.empty()){
        std::pop_heap(arr.begin(), arr.end(), cmp);
        auto ret = arr.back();
        arr.pop_back();
        if (!TIME_TICK) {
            assert(ret->val == *iter);
            iter++;
        }
    }
    GET_TIME_OF_DAY(&t1, NULL);
    if (TIME_TICK) {
        c_pop = tick(t1, t0);
        printf("STL heap. time cost: %llu ms\n"\
            "    push: %llu op / %llu us = %llu opus\n"\
            "    pop : %llu op / %llu us = %llu opus\n"\
            "    average  : %llu op / %llu us = %llu opus\n",
            (c_pop + c_push) / 1000,
            push_times, c_push, push_times / c_push,
            pop_times,  c_pop, pop_times / c_pop,
            push_times + pop_times, c_push + c_pop, (push_times + pop_times)/(c_push + c_pop)
            );
    }
};