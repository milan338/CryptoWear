#ifndef LOCK_H
#define LOCK_H

#include <eina_types.h>
#include <pthread.h>

typedef pthread_mutex_t Lock;

inline Eina_Bool lock_new(Lock *mutex)
{
    pthread_mutexattr_t attr;
    Eina_Bool ret = EINA_FALSE;
    if (pthread_mutexattr_init(&attr))
        return EINA_FALSE;
    if (!pthread_mutex_init(mutex, &attr))
        ret = EINA_TRUE;
    pthread_mutexattr_destroy(&attr);
    return ret;
}

inline void lock_free(Lock *mutex)
{
    pthread_mutex_destroy(mutex);
}

inline int lock_take(Lock *mutex)
{
    return pthread_mutex_lock(mutex);
}

inline int lock_release(Lock *mutex)
{
    return pthread_mutex_unlock(mutex);
}

#endif // LOCK_H
