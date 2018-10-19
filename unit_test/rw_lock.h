#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <unistd.h>

// 并发读写请求，优先响应读
class SharedLock
{
public:
    
    SharedLock(): m_wait_read(0), m_wait_write(0), m_readers(0){};

    SharedLock& operator = (const SharedLock &obj) = delete;
    SharedLock(const SharedLock &obj) = delete;
    
    void lock_shared()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_wait_read++;
        m_rd_cond.wait(lock, [&](){ 
            return (m_wait_write.load() == 0) && (m_status.load() != WR_LOCK); 
        });
        m_wait_read--;
        m_readers++;
        m_status.store(1);
    }

    void lock()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_wait_write++;
        m_wr_cond.wait(lock, [&](){
            return m_status.load() == NO_LOCK;
        });
        m_wait_write--;
        m_status = WR_LOCK;
    }

    void unlock_shared()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        assert(m_status.load() == 1);
        m_readers--;
        if (m_readers.load() == 0 && m_wait_read.load() == 0 && 
            m_wait_write.load() > 0 /* 没有读请求时再响应写 */) {
            m_wr_cond.notify_one();
            return;
        }
        if (m_wait_read.load() > 0) {
            m_rd_cond.notify_all();
            return;
        }
        if (m_readers.load() == 0) m_status.store(NO_LOCK);
    }

    void unlock()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        assert(m_status.load() == -1);
        m_status.store(NO_LOCK);
        if (m_wait_read.load()) {
            m_rd_cond.notify_all();
        }
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_rd_cond;
    std::condition_variable m_wr_cond;
    std::atomic_int m_wait_read;
    std::atomic_int m_readers;
    std::atomic_int m_wait_write;
    std::atomic_int m_status;
    const static int RD_LOCK = 1;
    const static int WR_LOCK = -1;
    const static int NO_LOCK = 0;
};

// RAII 封装
class RWLock
{
public:
    static const int READ = 0;
    static const int WRITE = 0;

    RWLock(){};

    RWLock(SharedLock &mtx, int type = 0)
    {
        assert(type == RWLock::READ || type == RWLock::WRITE);
        m_mtx = &mtx;
        m_type = type;
        if (m_type == 0){
            m_mtx->lock_shared();
        } else {
            m_mtx->lock();
        }
    }
    
    ~RWLock()
    {
        if (m_type == 0) {
            m_mtx->unlock_shared();
        } else {
            m_mtx->unlock();
        }
    }
private:
    SharedLock *m_mtx;
    int m_type;
};