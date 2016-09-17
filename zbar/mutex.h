#ifndef _ZBAR_MUTEX_H_
#define _ZBAR_MUTEX_H_

#include <config.h>

/* simple platform mutex abstraction
 */

#if defined(_WIN32)

# include <windows.h>

# define DEBUG_LOCKS
# ifdef DEBUG_LOCKS

typedef struct zbar_mutex_s {
    int count;
    CRITICAL_SECTION mutex;
} zbar_mutex_t;

static inline int _zbar_mutex_init (zbar_mutex_t *lock) {
    lock->count = 1;
    InitializeCriticalSection(&lock->mutex);
    return(0);
}

static inline void _zbar_mutex_destroy (zbar_mutex_t *lock) {
    DeleteCriticalSection(&lock->mutex);
}

static inline int _zbar_mutex_lock (zbar_mutex_t *lock) {
    EnterCriticalSection(&lock->mutex);
    if(lock->count++ < 1)
        assert(0);
    return(0);
}

static inline int _zbar_mutex_unlock (zbar_mutex_t *lock) {
    if(lock->count-- <= 1)
        assert(0);
    LeaveCriticalSection(&lock->mutex);
    return(0);
}

# else

typedef CRITICAL_SECTION zbar_mutex_t;

static inline int _zbar_mutex_init (zbar_mutex_t *lock) {
    InitializeCriticalSection(lock);
    return(0);
}

static inline void _zbar_mutex_destroy (zbar_mutex_t *lock) {
    DeleteCriticalSection(lock);
}

static inline int _zbar_mutex_lock (zbar_mutex_t *lock) {
    EnterCriticalSection(lock);
    return(0);
}

static inline int _zbar_mutex_unlock (zbar_mutex_t *lock) {
    LeaveCriticalSection(lock);
    return(0);
}

# endif


#elif defined(HAVE_LIBPTHREAD)

# include <pthread.h>

typedef pthread_mutex_t zbar_mutex_t;

static inline int _zbar_mutex_init (zbar_mutex_t *lock) {
# ifdef DEBUG_LOCKS
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    int rc = pthread_mutex_init(lock, &attr);
    pthread_mutexattr_destroy(&attr);
    return(rc);
# else
    return(pthread_mutex_init(lock, NULL));
# endif
}

static inline void _zbar_mutex_destroy (zbar_mutex_t *lock) {
    pthread_mutex_destroy(lock);
}

static inline int _zbar_mutex_lock (zbar_mutex_t *lock) {
    int rc = pthread_mutex_lock(lock);
# ifdef DEBUG_LOCKS
    assert(!rc);
# endif
    /* FIXME save system code */
    /*rc = err_capture(proc, SEV_ERROR, ZBAR_ERR_LOCKING, __func__,
                       "unable to lock processor");*/
    return(rc);
}

static inline int _zbar_mutex_unlock (zbar_mutex_t *lock) {
    int rc = pthread_mutex_unlock(lock);
# ifdef DEBUG_LOCKS
    assert(!rc);
# endif
    /* FIXME save system code */
    return(rc);
}


#else

typedef int zbar_mutex_t[0];

#define _zbar_mutex_init(l) -1
#define _zbar_mutex_destroy(l)
#define _zbar_mutex_lock(l) 0
#define _zbar_mutex_unlock(l) 0

#endif

#endif
