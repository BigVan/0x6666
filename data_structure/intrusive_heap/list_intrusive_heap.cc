#include "heaptest.h"

class heap_node;
struct heap_op {
	int op;
	heap_node *v;
};


class link_node
{
public:
    link_node *prev = nullptr;
    link_node *next = nullptr;
    static void swap_node(link_node *a, link_node *b)
    {
        if (a->next == b){
            assert(b->prev == a);
            /* prev_a <--> a <--> b <--> nt_b */
            if (a->prev) a->prev->push_back(b);
            else b->prev = nullptr;
            a->push_back(b->next);
            b->push_back(a);
            return;
        }
        auto tmp_prev = a->prev;
        if (b->prev) {
            b->prev->push_back(a);
        } else {
            a->prev = nullptr;
        }
        if (tmp_prev) {
            tmp_prev->push_back(b);
        } else {
            b->prev = nullptr;
        }
        auto tmp_next = a->next;
        a->push_back(b->next);
        b->push_back(tmp_next);
    }
    void push_back(link_node *obj)
    {
        this->next = obj;
        if (obj == nullptr) return;
        obj->prev = this;
    }
    link_node* pop_next()
    {
        auto ret = this->next;
        this->next = nullptr;
        if (ret == nullptr) return ret;
        ret->prev = nullptr;
        ret->next = nullptr;
        return ret;
    }
};

template<typename T>
class intrusive_heap
{
public:
    class node : public link_node
    {
    public:
        void set_parent(node *parent, bool tag)
        {
            this->remove_parent();
            if (parent == nullptr) return;
            if (tag == LEFT_TAG){
                parent->set_left_child(this);
            } else {
                parent->set_right_child(this);
            }
        }

        void set_left_child(node *child)
        {
           this->left = child;
           if (child == nullptr) return;
           child->parent = this; 
           child->tag = LEFT_TAG;
        }
        
        void set_right_child(node *child)
        {
            this->right = child;
            if (child == nullptr) return;
            child->parent = this;
            child->tag = RIGHT_TAG;
        }

        void remove_child(bool tag)
        {
            if (tag == LEFT_TAG){
                (this->left)->parent = nullptr;
                this->left = nullptr;
            } else {
                (this->right)->parent = nullptr;
                this->right = nullptr;
            }
        }

        void remove_parent()
        {
            if (!this->parent) return;
            if (this->tag == LEFT_TAG){
                this->parent->left = nullptr;
            } else {
                this->parent->right = nullptr;
            }
            this->parent = nullptr;
        }

        bool operator < (const node& rhs) const
        {
            return (*(T*)this) < (*(T*)&rhs);
        } 
        
        node *left = nullptr, *right = nullptr, *parent = nullptr;
        
        bool tag = LEFT_TAG; // 0 means it's a left child, 1 means right

    };

    void push(node *obj)
    {
        count++;
        if (top == nullptr) {
            top = last = obj;
            return;
        }
        last->push_back(obj);
        locate_parent(obj);
        last = obj;
        heap_up(obj);
    }

    node* pop()
    {
        if (count == 0) return nullptr;
        count--;
        auto ret = top;
        if (top == last) {
            top = last = nullptr;
            return ret;
        }
        auto prev = last->prev;
        auto top_l = top->left;
        auto top_r = top->right;
        last->remove_parent();
        last = static_cast<node *>(prev);
        auto obj = static_cast<node *>(last->pop_next());
        if (count > 2){
             obj->set_right_child(top_r);
        }
        if (count > 1) {
            obj->set_left_child(top_l);
            obj->push_back(top_l);
        } else {
            top = last = obj;
            return ret;
        }
        heap_down(obj);
        return ret;
    }

    void erase(node *obj)
    {
        if (obj == nullptr) return;
        count--;
       
        if (last == top){
            last = top = nullptr;
            return;
        }
        last = static_cast<node *>(last->prev);
        auto tmp = static_cast<node *>(last->pop_next());
        (tmp->parent)->remove_child(tmp->tag);   
        if (obj == tmp) return;
        link_node::swap_node(obj, tmp);
        tmp->set_parent(obj->parent, obj->tag);
        tmp->set_left_child(obj->left);
        tmp->set_right_child(obj->right);
        if (tmp->parent == nullptr) top = tmp;
        if (tmp->next == nullptr) last = tmp;
        heap_up(tmp);
        heap_down(tmp);
    }

    void print()
    {
        for (auto ptr = top; ptr != nullptr; ptr = static_cast<node *>(ptr->next)){
            print_info(ptr);
        }
    }

    bool empty() const { return count == 0; }
    ssize_t size() const { return count; }
    node *top = nullptr, *last = nullptr;

private:

    const static uint8_t LEFT_TAG = 0;
    const static uint8_t RIGHT_TAG = 1;
    ssize_t count = 0;
   
    void heap_up(node *obj)
    {
        auto ptr = obj->parent;
        while (ptr != nullptr) {
            if (*obj < *ptr) {
                swap_node(ptr, obj);
            } else {
                break;
            }
            ptr = obj->parent;
        }
    }

    void heap_down(node *obj)
    {
        while (true) {
            auto ptr = obj->left;
            if (obj->right && (ptr == nullptr || 
                    *(obj->right) < *(obj->left)) ) 
            {
                ptr = obj->right;
            }
            if (ptr == nullptr || *obj < *ptr) {
                break;
            } else {
                swap_node(obj, ptr);
            } 
        }
        check_top(obj);
    }

    void swap_node(node *parent, node *child)
    {
        auto tmp_l = parent->left;
        auto tmp_r = parent->right;
        auto tag = child->tag;
        link_node::swap_node(parent, child);
        if (parent->next == nullptr)
            last = parent;
        child->set_parent(parent->parent, parent->tag);
        parent->set_left_child(child->left);
        parent->set_right_child(child->right);
        if (tag == LEFT_TAG) {
            child->set_left_child(parent);
            child->set_right_child(tmp_r);
        } else {
            child->set_left_child(tmp_l);
            child->set_right_child(parent);
        }
        check_top(child);
    }

    inline void locate_parent(const node *obj)
    {
        auto prev = static_cast<node *>(obj->prev);
        assert(prev != nullptr);
        if (top == prev){
            top->set_left_child((node*)obj);
            return;
        }
        auto p = prev->parent;
        if (prev->tag == LEFT_TAG) {
            p->set_right_child((node *)obj);
            return;
        }
        p = static_cast<node *>(p->next);
        p->set_left_child((node *)obj);
    }
    
    inline void check_top(const node* obj)
    {
        if (obj->parent != nullptr) return;
        assert(obj->prev == nullptr);
        top = const_cast<node *>(obj);
    };
};

class heap_node : public intrusive_heap<heap_node>::node
{
public:
    int val;
    bool operator <(const heap_node &obj)
    {
        return this->val < obj.val;
    }
  
    heap_node(int v) : val(v){};
};

TEST_F(Heap, listBaseErase)
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
            pushs.push_back(heap_op{ Heap::PUSH, new heap_node(pair.second) });
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
    
    intrusive_heap<heap_node> heap;
    gettimeofday(&t0, NULL);
    for (auto it : pushs){
        heap.push(it.v);
    }
    gettimeofday(&t1, NULL);
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

    gettimeofday(&t0, NULL);
    for (auto it : erases){
        heap.erase(it.v);
    }
    gettimeofday(&t1, NULL);
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
    
    std::set<heap_node *> set;
    gettimeofday(&t0, NULL);
    for (auto it : pushs){
        set.insert(it.v);
    }
    gettimeofday(&t1, NULL);
    c_push = tick(t1, t0);
    gettimeofday(&t0, NULL);
    for (auto it : erases){
        set.erase(it.v);
    }
    gettimeofday(&t1, NULL);
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

TEST_F(Heap, listBaseSort)
{
    int count = COUNT, rand_limit = RAND_LIMIT;
    std::vector<heap_op> pushs, pops;

    auto make_test = [&](std::vector<heap_op> &pushs, std::vector<heap_op> &pops)
    {
        for (auto pair : this->pairs){
            if ( pair.first == Heap::POP) {
                pops.push_back(heap_op{ 1 });
                continue;
            }
            pushs.push_back(heap_op{ Heap::PUSH, new heap_node(pair.second) });
        }
        return;
    };
    make_test(pushs, pops);
   
    intrusive_heap<heap_node> heap;
    std::vector<int> result;
    result.reserve(count);
    
    struct timeval t0, t1;
    uint64_t c_push = 0, c_pop = 0;
    uint64_t push_times = pushs.size(), pop_times = pushs.size();
    auto tick = [](const timeval &t2, const timeval &t1){
        return (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    };
    gettimeofday(&t0, NULL);
    for (auto it : pushs){
        heap.push(it.v);
    }
    gettimeofday(&t1, NULL);
    c_push = tick(t1, t0);

    gettimeofday(&t0, NULL);
    while (!heap.empty()){
        auto ret = heap.pop();
    }
    gettimeofday(&t1, NULL);

    c_pop = tick(t1, t0);
    printf("Intrusive Heap. time cost: %llu ms\n"\
           "    push: %llu op / %llu us = %llu opus\n"\
           "    pop : %llu op / %llu us = %llu opus\n"\
           "    average  : %llu op / %llu us = %llu opus\n",
           (c_pop + c_push) / 1000,
           push_times, c_push, push_times / c_push,
           pop_times,  c_pop, pop_times / c_pop,
           push_times + pop_times, c_push + c_pop, (push_times + pop_times)/(c_push + c_pop)
        );
    
    std::vector<heap_node *> arr;
    arr.reserve(push_times);
    auto cmp = [](heap_node *a, heap_node *b){
        return a->val >= b->val;
    };
    std::make_heap(arr.begin(), arr.end(), cmp );
    gettimeofday(&t0, NULL);
    for (auto it : pushs){
        arr.push_back(it.v);
        std::push_heap(arr.begin(), arr.end(), cmp);
    }   
    gettimeofday(&t1, NULL);
    c_push = tick(t1, t0);
    gettimeofday(&t0, NULL);
    while (!arr.empty()){
        std::pop_heap(arr.begin(), arr.end(), cmp);
        arr.pop_back();
    }
    gettimeofday(&t1, NULL);

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