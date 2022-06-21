#include <eina_types.h>
#include <pthread.h>
#include "lock.h"

extern inline Eina_Bool lock_new(Lock *mutex);
extern inline void lock_free(Lock *mutex);
extern inline int lock_take(Lock *mutex);
extern inline int lock_release(Lock *mutex);
