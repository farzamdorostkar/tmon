#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <cstdint>
#include <cstdlib>  // For exit and EXIT_FAILURE
#include "../../llvm-project-17.0.6.src/llvm/lib/Transforms/Instrumentation/ptwrite_instrumentation.h"

#ifdef TMON_DEBUG_MODE
#define DPrintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DPrintf(...)
#endif

#define FUNC_TYPE(x) x##_type

#define PTR_TO_REAL(x) real_##x

#define DEFINE_REAL(ret_type, func, ...) \
  typedef ret_type (*FUNC_TYPE(func))(__VA_ARGS__); \
  namespace __interception {                        \
  FUNC_TYPE(func) PTR_TO_REAL(func) = NULL;         \
  }

#define DECLARE_WRAPPER(ret_type, func, ...) \
  extern "C" ret_type func(__VA_ARGS__)      \
  __attribute__((weak, alias("__interceptor_" #func), visibility("default")));

#define INTERCEPTOR_ATTRIBUTE __attribute__((visibility("default")))

#define WRAP(x) __interceptor_ ## x

#define TMON_INTERCEPTOR(ret_type, func, ...)  \
  DEFINE_REAL(ret_type, func, __VA_ARGS__)     \
  DECLARE_WRAPPER(ret_type, func, __VA_ARGS__) \
  extern "C"                                   \
  INTERCEPTOR_ATTRIBUTE                        \
  ret_type WRAP(func)(__VA_ARGS__)

// Get the real function from the dynamic linker
template <typename FuncType>
FuncType get_real_function(const char *func_name) {
  FuncType real_func = reinterpret_cast<FuncType>(dlsym(RTLD_NEXT, func_name));
  if (!real_func) {
    fprintf(stderr, "Failed to obtain real %s\n", func_name);
    exit(EXIT_FAILURE);
  }
  return real_func;
}

struct ThreadParam {
  void* (*callback)(void *arg);
  void *param;
};

extern "C" void *__tmon_thread_start_func(void *arg) {
  DPrintf("__tmon_thread_start_func\n");

  ThreadParam *p = static_cast<ThreadParam*>(arg);
  void* (*callback)(void *arg) = p->callback;
  void *param = p->param;

  // Instrument start routine
  pthread_t id = pthread_self();
  asm volatile ("ptwriteq %0"::"irm"(THREAD_START_MASK | (id & 0xffffffffffff)));

  void *res = callback(param);

  //delete p;
  free(p);

  return res;
}

// #1
TMON_INTERCEPTOR(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr, void *(*callback)(void *), void *arg) {
  DPrintf("__interceptor_pthread_create\n");

  __interception::real_pthread_create = get_real_function<pthread_create_type>("pthread_create");

  ThreadParam *p = (ThreadParam *)calloc(1, sizeof(*p));
  p->callback = callback;
  p->param = arg;

  // Call the real pthread_create function
  int res = __interception::real_pthread_create(thread, attr, __tmon_thread_start_func, p);

  // Instrument pthread_create
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_CREATE | (((uint64_t)(res & 0xff)) << 48) | ((*thread) & 0xffffffffffff)));

  return res;
};

// #2
TMON_INTERCEPTOR(int, pthread_join, pthread_t thread, void **retval) {
  DPrintf("__interceptor_pthread_join\n");

  __interception::real_pthread_join = get_real_function<pthread_join_type>("pthread_join");

  // Call the real pthread_join function
  int res = __interception::real_pthread_join(thread, retval);

  // Instrument pthread_join
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_JOIN | (((uint64_t)(res & 0xff)) << 48) | (thread & 0xffffffffffff)));

  return res;
}

// #3
TMON_INTERCEPTOR(int, pthread_detach, pthread_t thread) {
  DPrintf("__interceptor_pthread_detach\n");

  __interception::real_pthread_detach = get_real_function<pthread_detach_type>("pthread_detach");

  // Call the real pthread_detach function
  int res = __interception::real_pthread_detach(thread);

  // Instrument pthread_detach
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_DETACH | (((uint64_t)(res & 0xff)) << 48) | (thread & 0xffffffffffff)));

  return res;
}

// #4
TMON_INTERCEPTOR(void, pthread_exit, void *retval) {
  DPrintf("__interceptor_pthread_exit\n");

  __interception::real_pthread_exit = get_real_function<pthread_exit_type>("pthread_exit");

  __interception::real_pthread_exit(retval);

  asm volatile ("ptwriteq %0"::"rm"(PTHREAD_EXIT));

  // Note: This function returns void.
}

// #5
TMON_INTERCEPTOR(int, pthread_tryjoin_np, pthread_t thread, void** retval) {
  DPrintf("__interceptor_pthread_tryjoin_np\n");

  __interception::real_pthread_tryjoin_np = get_real_function<pthread_tryjoin_np_type>("pthread_tryjoin_np");

  int res = __interception::real_pthread_tryjoin_np(thread, retval);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_TRYJOIN_NP | (((uint64_t)(res & 0xff)) << 48) | (thread & 0xffffffffffff)));

  return res;
}

// #6
TMON_INTERCEPTOR(int, pthread_timedjoin_np, pthread_t thread, void **retval, const struct timespec *abstime) {
  DPrintf("__interceptor_pthread_timedjoin_np\n");

  __interception::real_pthread_timedjoin_np = get_real_function<pthread_timedjoin_np_type>("pthread_timedjoin_np");

  // Call the real pthread_timedjoin_np function
  int res = __interception::real_pthread_timedjoin_np(thread, retval, abstime);

  // Instrument pthread_timedjoin_np
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_TIMEDJOIN_NP | (((uint64_t)(res & 0xff)) << 48) | (thread & 0xffffffffffff)));

  return res;
}

// #7
TMON_INTERCEPTOR(int, pthread_cond_init, pthread_cond_t* cond, const pthread_condattr_t* attr) {
  DPrintf("__interceptor_pthread_cond_init\n");

  __interception::real_pthread_cond_init = get_real_function<pthread_cond_init_type>("pthread_cond_init");

  int res = __interception::real_pthread_cond_init(cond, attr);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_COND_INIT | ((uintptr_t)cond & 0xffffffffffff)));

  return res;
}

// #8
TMON_INTERCEPTOR(int, pthread_cond_wait, pthread_cond_t* cond, pthread_mutex_t* mutex) {
  DPrintf("__interceptor_pthread_cond_wait\n");

  __interception::real_pthread_cond_wait = get_real_function<pthread_cond_wait_type>("pthread_cond_wait");

  // Call the real pthread_cond_wait function
  int res = __interception::real_pthread_cond_wait(cond, mutex);

  // Instrument pthread_cond_wait
  // TODO

  return res;
}

// #9
TMON_INTERCEPTOR(int, pthread_cond_timedwait, pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime) {
  DPrintf("__interceptor_pthread_cond_timedwait\n");

  __interception::real_pthread_cond_timedwait = get_real_function<pthread_cond_timedwait_type>("pthread_cond_timedwait");

  // Call the real pthread_cond_timedwait function
  int res = __interception::real_pthread_cond_timedwait(cond, mutex, abstime);

  // Instrument pthread_cond_timedwait
  // TODO

  return res;
}

// #10 TODO: double check the signature
TMON_INTERCEPTOR(int, pthread_cond_clockwait, pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clockid, const struct timespec *abstime) {
  DPrintf("__interceptor_pthread_cond_clockwait\n");

  __interception::real_pthread_cond_clockwait = get_real_function<pthread_cond_clockwait_type>("pthread_cond_clockwait");

  // Call the real pthread_cond_clockwait function
  int res = __interception::real_pthread_cond_clockwait(cond, mutex, clockid, abstime);

  // Instrument pthread_cond_clockwait
  // TODO

  return res;
}

// #11
TMON_INTERCEPTOR(int, pthread_cond_signal, pthread_cond_t *cond) {
  DPrintf("__interceptor_pthread_cond_signal\n");

  __interception::real_pthread_cond_signal = get_real_function<pthread_cond_signal_type>("pthread_cond_signal");

  int res = __interception::real_pthread_cond_signal(cond);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_COND_SIGNAL | ((uintptr_t)cond & 0xffffffffffff)));

  return res;
}

// #12
TMON_INTERCEPTOR(int, pthread_cond_broadcast, pthread_cond_t *cond) {
  DPrintf("__interceptor_pthread_cond_broadcast\n");

  __interception::real_pthread_cond_broadcast = get_real_function<pthread_cond_broadcast_type>("pthread_cond_broadcast");

  int res = __interception::real_pthread_cond_broadcast(cond);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_COND_BROADCAST | ((uintptr_t)cond & 0xffffffffffff)));

  return res;
}

// #13
TMON_INTERCEPTOR(int, pthread_cond_destroy, pthread_cond_t *cond) {
  DPrintf("__interceptor_pthread_cond_destroy\n");

  __interception::real_pthread_cond_destroy = get_real_function<pthread_cond_destroy_type>("pthread_cond_destroy");

  int res = __interception::real_pthread_cond_destroy(cond);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_COND_DESTROY | ((uintptr_t)cond & 0xffffffffffff)));

  return res;
}

// #14
TMON_INTERCEPTOR(int, pthread_mutex_init, pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  DPrintf("__interceptor_pthread_mutex_init\n");

  __interception::real_pthread_mutex_init = get_real_function<pthread_mutex_init_type>("pthread_mutex_init");

  int res = __interception::real_pthread_mutex_init(mutex, attr);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_INIT | (((uint64_t)(res & 0xff)) << 48) | ((uintptr_t)mutex & 0xffffffffffff)));

  return res;
}

// #15
TMON_INTERCEPTOR(int, pthread_mutex_destroy, pthread_mutex_t *mutex) {
  DPrintf("__interceptor_pthread_mutex_destroy\n");

  __interception::real_pthread_mutex_destroy = get_real_function<pthread_mutex_destroy_type>("pthread_mutex_destroy");

  int res = __interception::real_pthread_mutex_destroy(mutex);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_DESTROY | (((uint64_t)(res & 0xff)) << 48) | ((uintptr_t)mutex & 0xffffffffffff)));

  return res;
}

// #16
TMON_INTERCEPTOR(int, pthread_mutex_trylock, pthread_mutex_t *mutex) {
  DPrintf("__interceptor_pthread_mutex_trylock\n");

  __interception::real_pthread_mutex_trylock = get_real_function<pthread_mutex_trylock_type>("pthread_mutex_trylock");

  int res = __interception::real_pthread_mutex_trylock(mutex);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_TRYLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));

  return res;
}

// #17
TMON_INTERCEPTOR(int, pthread_mutex_timedlock, pthread_mutex_t *mutex, const struct timespec *abstime) {
  DPrintf("__interceptor_pthread_mutex_timedlock\n");

  __interception::real_pthread_mutex_timedlock = get_real_function<pthread_mutex_timedlock_type>("pthread_mutex_timedlock");

  int res = __interception::real_pthread_mutex_timedlock(mutex, abstime);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_TIMEDLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));

  return res;
}

// #18
TMON_INTERCEPTOR(int, pthread_spin_init, pthread_spinlock_t *lock, int pshared) {
  DPrintf("__interceptor_pthread_spin_init\n");

  __interception::real_pthread_spin_init = get_real_function<pthread_spin_init_type>("pthread_spin_init");

  int res = __interception::real_pthread_spin_init(lock, pshared);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_SPIN_INIT | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)lock) & 0xffffffffffff)));

  return res;
}

// #19
TMON_INTERCEPTOR(int, pthread_spin_destroy, pthread_spinlock_t *lock) {
  DPrintf("__interceptor_pthread_spin_destroy\n");

  __interception::real_pthread_spin_destroy = get_real_function<pthread_spin_destroy_type>("pthread_spin_destroy");

  int res = __interception::real_pthread_spin_destroy(lock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_SPIN_DESTROY | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)lock) & 0xffffffffffff)));

  return res;
}

// #20
TMON_INTERCEPTOR(int, pthread_spin_lock, pthread_spinlock_t *lock) {
  DPrintf("__interceptor_pthread_spin_lock\n");

  __interception::real_pthread_spin_lock = get_real_function<pthread_spin_lock_type>("pthread_spin_lock");

  int res = __interception::real_pthread_spin_lock(lock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_SPIN_LOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)lock) & 0xffffffffffff)));

  return res;
}

// #21
TMON_INTERCEPTOR(int, pthread_spin_trylock, pthread_spinlock_t *lock) {
  DPrintf("__interceptor_pthread_spin_trylock\n");

  __interception::real_pthread_spin_trylock = get_real_function<pthread_spin_trylock_type>("pthread_spin_trylock");

  int res = __interception::real_pthread_spin_trylock(lock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_SPIN_TRYLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)lock) & 0xffffffffffff)));

  return res;
}

// #22
TMON_INTERCEPTOR(int, pthread_spin_unlock, pthread_spinlock_t *lock) {
  DPrintf("__interceptor_pthread_spin_unlock\n");

  __interception::real_pthread_spin_unlock = get_real_function<pthread_spin_unlock_type>("pthread_spin_unlock");

  int res = __interception::real_pthread_spin_unlock(lock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_SPIN_UNLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)lock) & 0xffffffffffff)));

  return res;
}

// #23
TMON_INTERCEPTOR(int, pthread_rwlock_init, pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr) {
  DPrintf("__interceptor_pthread_rwlock_init\n");

  __interception::real_pthread_rwlock_init = get_real_function<pthread_rwlock_init_type>("pthread_rwlock_init");

  int res = __interception::real_pthread_rwlock_init(rwlock, attr);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_INIT | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #24
TMON_INTERCEPTOR(int, pthread_rwlock_destroy, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_destroy\n");

  __interception::real_pthread_rwlock_destroy = get_real_function<pthread_rwlock_destroy_type>("pthread_rwlock_destroy");

  int res = __interception::real_pthread_rwlock_destroy(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_DESTROY | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #25
TMON_INTERCEPTOR(int, pthread_rwlock_rdlock, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_rdlock\n");

  __interception::real_pthread_rwlock_rdlock = get_real_function<pthread_rwlock_rdlock_type>("pthread_rwlock_rdlock");

  int res = __interception::real_pthread_rwlock_rdlock(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_RDLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #26
TMON_INTERCEPTOR(int, pthread_rwlock_tryrdlock, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_tryrdlock\n");

  __interception::real_pthread_rwlock_tryrdlock = get_real_function<pthread_rwlock_tryrdlock_type>("pthread_rwlock_tryrdlock");

  int res = __interception::real_pthread_rwlock_tryrdlock(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_TRYRDLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #27
TMON_INTERCEPTOR(int, pthread_rwlock_timedrdlock, pthread_rwlock_t *rwlock, const struct timespec *abstime) {
  DPrintf("__interceptor_pthread_rwlock_timedrdlock\n");

  __interception::real_pthread_rwlock_timedrdlock = get_real_function<pthread_rwlock_timedrdlock_type>("pthread_rwlock_timedrdlock");

  int res = __interception::real_pthread_rwlock_timedrdlock(rwlock, abstime);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_TIMEDRDLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #28
TMON_INTERCEPTOR(int, pthread_rwlock_wrlock, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_wrlock\n");

  __interception::real_pthread_rwlock_wrlock = get_real_function<pthread_rwlock_wrlock_type>("pthread_rwlock_wrlock");

  int res = __interception::real_pthread_rwlock_wrlock(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_WRLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #29
TMON_INTERCEPTOR(int, pthread_rwlock_trywrlock, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_trywrlock\n");

  __interception::real_pthread_rwlock_trywrlock = get_real_function<pthread_rwlock_trywrlock_type>("pthread_rwlock_trywrlock");

  int res = __interception::real_pthread_rwlock_trywrlock(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_TRYWRLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #30
TMON_INTERCEPTOR(int, pthread_rwlock_timedwrlock, pthread_rwlock_t *rwlock, const struct timespec *abstime) {
  DPrintf("__interceptor_pthread_rwlock_timedwrlock\n");

  __interception::real_pthread_rwlock_timedwrlock = get_real_function<pthread_rwlock_timedwrlock_type>("pthread_rwlock_timedwrlock");

  int res = __interception::real_pthread_rwlock_timedwrlock(rwlock, abstime);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_TIMEDWRLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #31
TMON_INTERCEPTOR(int, pthread_rwlock_unlock, pthread_rwlock_t *rwlock) {
  DPrintf("__interceptor_pthread_rwlock_unlock\n");

  __interception::real_pthread_rwlock_unlock = get_real_function<pthread_rwlock_unlock_type>("pthread_rwlock_unlock");

  int res = __interception::real_pthread_rwlock_unlock(rwlock);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_RWLOCK_UNLOCK | (((uint64_t)rwlock) & 0xffffffffffff)));

  return res;
}

// #32
TMON_INTERCEPTOR(int, pthread_barrier_init, pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count) {
  DPrintf("__interceptor_pthread_barrier_init\n");

  __interception::real_pthread_barrier_init = get_real_function<pthread_barrier_init_type>("pthread_barrier_init");

  int res = __interception::real_pthread_barrier_init(barrier, attr, count);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_BARRIER_INIT | (((uint64_t)barrier) & 0xffffffffffff)));

  return res;
}

// #33
TMON_INTERCEPTOR(int, pthread_barrier_destroy, pthread_barrier_t *barrier) {
  DPrintf("__interceptor_pthread_barrier_destroy\n");

  __interception::real_pthread_barrier_destroy = get_real_function<pthread_barrier_destroy_type>("pthread_barrier_destroy");

  int res = __interception::real_pthread_barrier_destroy(barrier);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_BARRIER_DESTROY | (((uint64_t)barrier) & 0xffffffffffff)));

  return res;
}

// #34
TMON_INTERCEPTOR(int, pthread_barrier_wait, pthread_barrier_t *barrier) {
  DPrintf("__interceptor_pthread_barrier_wait\n");

  __interception::real_pthread_barrier_wait = get_real_function<pthread_barrier_wait_type>("pthread_barrier_wait");

  int res = __interception::real_pthread_barrier_wait(barrier);

  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_BARRIER_WAIT | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)barrier) & 0xffffffffffff)));

  return res;
}

// #35
//TODO: pthread_once

// #36
TMON_INTERCEPTOR(int, pthread_sigmask, int how, const sigset_t *set, sigset_t *oldset) {
  DPrintf("__interceptor_pthread_sigmask\n");
  __interception::real_pthread_sigmask = get_real_function<pthread_sigmask_type>("pthread_sigmask");
  int res = __interception::real_pthread_sigmask(how, set, oldset);
  asm volatile ("ptwriteq %0"::"rm"(PTHREAD_SIGMASK));
  return res;
}

// #37
TMON_INTERCEPTOR(int, pthread_kill, pthread_t thread, int sig) {
  __interception::real_pthread_kill = get_real_function<pthread_kill_type>("pthread_kill");
  int res = __interception::real_pthread_kill(thread, sig);
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_KILL | (((uint64_t)sig) << 32) | (thread & 0xffffffffffff)));
  return res;
}


// Common Interceptor

// #if SANITIZER_INTERCEPT_PTHREAD_MUTEX
// #3
TMON_INTERCEPTOR(int, pthread_mutex_lock, pthread_mutex_t *mutex) {
  DPrintf("__interceptor_pthread_mutex_lock\n");
  __interception::real_pthread_mutex_lock = get_real_function<pthread_mutex_lock_type>("pthread_mutex_lock");
  int res = __interception::real_pthread_mutex_lock(mutex);
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_LOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));
  return res;
}

// #4
TMON_INTERCEPTOR(int, pthread_mutex_unlock, pthread_mutex_t *mutex) {
  DPrintf("__interceptor_pthread_mutex_unlock\n");
  __interception::real_pthread_mutex_unlock = get_real_function<pthread_mutex_unlock_type>("pthread_mutex_unlock");
  int res = __interception::real_pthread_mutex_unlock(mutex);
  asm volatile ("ptwriteq %0"::"irm"(PTHREAD_MUTEX_UNLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));
  return res;
}
//#endif

//#if SANITIZER_INTERCEPT___PTHREAD_MUTEX
//#5
TMON_INTERCEPTOR(int, __pthread_mutex_lock, pthread_mutex_t *mutex) {
  DPrintf("__interceptor___pthread_mutex_lock\n");
  __interception::real___pthread_mutex_lock = get_real_function<__pthread_mutex_lock_type>("__pthread_mutex_lock");
  int res = __interception::real___pthread_mutex_lock(mutex);
  asm volatile ("ptwriteq %0"::"irm"(__PTHREAD_MUTEX_LOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));
  return res;
}

// #6
TMON_INTERCEPTOR(int, __pthread_mutex_unlock, pthread_mutex_t *mutex) {
  DPrintf("__interceptor___pthread_mutex_unlock\n");
  __interception::real___pthread_mutex_unlock = get_real_function<__pthread_mutex_unlock_type>("__pthread_mutex_unlock");
  int res = __interception::real___pthread_mutex_unlock(mutex);
  asm volatile ("ptwriteq %0"::"irm"(__PTHREAD_MUTEX_UNLOCK | (((uint64_t)(res & 0xff)) << 48) | (((uint64_t)mutex) & 0xffffffffffff)));
  return res;
}
//#endif