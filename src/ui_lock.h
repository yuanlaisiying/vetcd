#ifndef __LY_UI_LOCK_H__
#define __LY_UI_LOCK_H__

class UiLock
{
public:
    UiLock()
    {
#if defined(_MSC_VER)
        InitializeCriticalSection(&cs_);
#else
        /* PTHREAD_MUTEX_RECURSIVE */
        pthread_mutexattr_init(&cs_attr_);
        pthread_mutexattr_settype(&cs_attr_, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&cs_, &cs_attr_);
#endif
    }
    ~UiLock()
    {
#if defined(_MSC_VER)
        DeleteCriticalSection(&cs_);
#else
        pthread_mutex_destroy(&cs_);
        pthread_mutexattr_destroy(&cs_attr_);
#endif
    }

public:
    void Lock()
    {
#if defined(_MSC_VER)
        EnterCriticalSection(&cs_);
#else
        pthread_mutex_lock(&cs_);
#endif
    }
    void Unlock()
    {
#if defined(_MSC_VER)
        LeaveCriticalSection(&cs_);
#else
        pthread_mutex_unlock(&cs_);
#endif
    }

private:
#if defined(_MSC_VER)
    CRITICAL_SECTION cs_;
#else
    pthread_mutex_t cs_;
    pthread_mutexattr_t cs_attr_;
#endif
};


#endif // __LY_UI_LOCK_H__