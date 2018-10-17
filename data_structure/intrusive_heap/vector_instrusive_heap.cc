
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
        auto ret = m_nodes[0];
        m_nodes[0] = m_nodes.back();
        m_nodes.pop_back();
        down(0);
        return ret;
    }

    void erase(node *obj)
    {
        if (obj->idx == m_nodes.size() - 1) {
            m_nodes.pop_back();
            return;
        }
        auto id = obj->idx;
        m_nodes[id] = m_nodes.back();
        m_nodes[id]->idx = id;
        m_nodes.pop_back();
        if (!down(id)) up(id);
    }

    void reserve(int size){ m_nodes.reserve(size); }

    size_t size(){ return m_nodes.size(); }

    bool empty() { return m_nodes.size() == 0; }

    void check()
    {
        for (auto i = 0; (i << 1) + 1 < m_nodes.size(); i++) {
            auto l = (i << 1) + 1;
            auto r = l + 1;
            assert(!(*m_nodes[l] < *m_nodes[i]));
            if (r < m_nodes.size())
                assert(!(*m_nodes[r] < *m_nodes[i]));
        }
    }

    intrusive_heap() 
    {
        m_update.resize(21); //level 21 (2^21 nodes);
    }

private:
    std::vector<node *> m_nodes;
    std::vector<int> m_update;
    int m_update_cnt = 0;

    bool up(int idx)
    {   
        auto tmp = m_nodes[idx];
        auto ret = false;
        while (idx != 0){
            auto cmpIdx = (idx - 1) >> 1;
            if (*tmp < *m_nodes[cmpIdx]){
                m_nodes[idx] = m_nodes[cmpIdx];
                m_nodes[idx]->idx = idx;
                ret = true;
                idx = cmpIdx;
                continue;
            } 
            break;
        }
        if (ret) {
            m_nodes[idx] = tmp;
            m_nodes[idx]->idx = idx;
        }
        return ret;
    }

    bool down(int idx)
    {
        auto tmp = m_nodes[idx];      
        auto cmpIdx = (idx << 1) + 1;
        auto ret = false;
        while (cmpIdx < m_nodes.size()) {
            if (cmpIdx + 1 < m_nodes.size() && 
                (*m_nodes[cmpIdx + 1]) < (*m_nodes[cmpIdx]))
            {
                cmpIdx++;
            }
            if (*m_nodes[cmpIdx] < *tmp){
                m_nodes[idx] = m_nodes[cmpIdx];
                m_nodes[idx]->idx = idx;
                ret = true;
                idx = cmpIdx;
                cmpIdx = (idx << 1) + 1;
                continue;
            } 
            break;
        }
        if (ret) {
            m_nodes[idx] = tmp;
            m_nodes[idx]->idx = idx;
        }
        return ret;
    }

    __attribute__((always_inline))
    void reset_update(int idx)
    {
        m_update_cnt = 1;
        m_update[0] = idx;
    }

    __attribute__((always_inline))
    void set_update(int idx)
    {
        m_update[m_update_cnt] = idx;
        m_update_cnt++;
    }

    __attribute__((always_inline))
    void move_nodes(node *_node)
    {
        auto p = 0;
        while (p + 1 < m_update_cnt){
            auto id_0 = m_update[p];
            auto id_1 = m_update[p + 1];
            m_nodes[id_0] = m_nodes[id_1];
            m_nodes[id_0]->idx = id_0;
            p++;
        }
        auto dest_id = m_update[p];
        m_nodes[dest_id] = _node;
        m_nodes[dest_id]->idx = dest_id;
    }
};


/* ================================================== */
/*                                                    */
/*   -------------- UNIT TEST BELOW  --------------   */
/*                                                    */
/* ================================================== */

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
    //heap.print();
    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);
    uint64_t erase_times = heap.size() / 2;
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
      //  printf("erase: %d %d\n", it.v->idx, it.v->val);
        heap.erase(it.v);
      
    }
    GET_TIME_OF_DAY(&t1, NULL);
    heap.check();
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
        return a->val > b->val;
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
        //printf("push: %d\n", it.v->val);
        heap.push(it.v);
      //  heap.print();
    }
    GET_TIME_OF_DAY(&t1, NULL);
    c_push = tick(t1, t0);
    std::vector<int> judge;
    GET_TIME_OF_DAY(&t0, NULL);
    while (!heap.empty()){
        auto ret = static_cast<intrusive_object *>(heap.pop());
        if (!TIME_TICK){
            printf("pop: %d\n", ret->val);
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