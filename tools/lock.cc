#include "lock.h"

class MutexLock : public ILock
{
public:

    MutexLock(){};
    MutexLock& operator = (const MutexLock &obj) = delete;
    MutexLock(const MutexLock &obj) = delete;

    virtual int lock() override
    {
        m_mtx.lock();
        return 0;
    }
    virtual int unlock() override
    {
        m_mtx.unlock();
        return 0;
    }
    virtual bool is_shared() const override 
    {
        return false;
    }

    UNIMPLEMENTED(int lock_shared() override);
    UNIMPLEMENTED(int unlock_shared() override);

protected:
    std::mutex m_mtx;
};

class SharedLock : public MutexLock
{
public:
    
    SharedLock(): m_readers(0){};
    SharedLock& operator = (const SharedLock &obj) = delete;
    SharedLock(const SharedLock &obj) = delete;
    
    int lock_shared() override
    {
        if (m_readers.fetch_add(1) == 0) m_mtx.lock();
        return 0;
    }
    int unlock_shared() override
    {
        if (m_readers.fetch_sub(1) == 1) m_mtx.unlock();
        return 0;
    }
    bool is_shared() const override 
    {
        return true;
    }

private:
    std::atomic_int m_readers;
};

ILock *create_shared_lock(){ return new SharedLock(); }
ILock *create_mutex_lock() { return new MutexLock(); }