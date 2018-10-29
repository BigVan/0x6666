#pragma once

#include <memory>
#include <mutex>
#include <unistd.h>
#include <assert.h>


#define UNIMPLEMENTED(func)  \
    virtual func             \
    {                        \
        errno = ENOSYS;      \
        printf("%s NO Implement\n", __FUNCTION__); \
        return -1;           \
    }

class ILock
{
public:
    virtual ~ILock(){};
    virtual int lock() = 0;
    virtual int unlock() = 0;
    virtual int lock_shared() = 0;
    virtual int unlock_shared() = 0;
    virtual bool is_shared() const = 0;
};

ILock *create_shared_lock();
ILock *create_mutex_lock();

// RAII 封装
class Lock
{
public:
    static const int READ       = 0;
    static const int WRITE      = 1;

    Lock(ILock *mtx, int type = WRITE) : m_mtx(mtx), m_type(type)
    {
        if (m_type == READ && m_mtx->is_shared()){
            m_mtx->lock_shared();
        } else {
            m_mtx->lock();
        }
    }
    
    ~Lock()
    {
        if (m_type == READ && m_mtx->is_shared()) {
            m_mtx->unlock_shared();
        } else {
            m_mtx->unlock();
        }
    }
private:
    ILock *m_mtx;
    int m_type;
};