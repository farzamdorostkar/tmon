#ifndef TMON_EH_H
#define TMON_EH_H

#include "common.h"

typedef void (*EventFunction)(synth_intel_ptwrite *);

void tmon_func_entry(synth_intel_ptwrite *event);
void tmon_function_exit(synth_intel_ptwrite *event);
void tmon_ignore_thread_begin(synth_intel_ptwrite *event);
void tmon_ignore_thread_end(synth_intel_ptwrite *event);
void tmon_read1(synth_intel_ptwrite *event);
void tmon_read2(synth_intel_ptwrite *event);
void tmon_read4(synth_intel_ptwrite *event);
void tmon_read8(synth_intel_ptwrite *event);
void tmon_read16(synth_intel_ptwrite *event);
void tmon_write1(synth_intel_ptwrite *event);
void tmon_write2(synth_intel_ptwrite *event);
void tmon_write4(synth_intel_ptwrite *event);
void tmon_write8(synth_intel_ptwrite *event);
void tmon_write16(synth_intel_ptwrite *event);
void tmon_unaligned_read1(synth_intel_ptwrite *event);
void tmon_unaligned_read2(synth_intel_ptwrite *event);
void tmon_unaligned_read4(synth_intel_ptwrite *event);
void tmon_unaligned_read8(synth_intel_ptwrite *event);
void tmon_unaligned_read16(synth_intel_ptwrite *event);
void tmon_unaligned_write1(synth_intel_ptwrite *event);
void tmon_unaligned_write2(synth_intel_ptwrite *event);
void tmon_unaligned_write4(synth_intel_ptwrite *event);
void tmon_unaligned_write8(synth_intel_ptwrite *event);
void tmon_unaligned_write16(synth_intel_ptwrite *event);
void tmon_vptr_read(synth_intel_ptwrite *event);
void tmon_vptr_write(synth_intel_ptwrite *event);
//
EventFunction tmon_atomic8_load;
EventFunction tmon_atomic16_load;
EventFunction tmon_atomic32_load;
EventFunction tmon_atomic64_load;
EventFunction tmon_atomic128_load;
EventFunction tmon_atomic8_store;
EventFunction tmon_atomic16_store;
EventFunction tmon_atomic32_store;
EventFunction tmon_atomic64_store;
EventFunction tmon_atomic128_store;
EventFunction tmon_atomic8_rmw;
EventFunction tmon_atomic16_rmw;
EventFunction tmon_atomic32_rmw;
EventFunction tmon_atomic64_rmw;
EventFunction tmon_atomic128_rmw;
EventFunction tmon_atomic8_cmpxchg;
EventFunction tmon_atomic16_cmpxchg;
EventFunction tmon_atomic32_cmpxchg;
EventFunction tmon_atomic64_cmpxchg;
EventFunction tmon_atomic128_cmpxchg;
// Tsan Specific Interceptors -------------------------------
void tmon_thread_start(synth_intel_ptwrite *event);
void tmon_pthread_create(synth_intel_ptwrite *event);
void tmon_pthread_join(synth_intel_ptwrite *event);
void tmon_pthread_detach(synth_intel_ptwrite *event);
void tmon_pthread_exit(synth_intel_ptwrite *event);
void tmon_pthread_tryjoin_np(synth_intel_ptwrite *event);
void tmon_pthread_timedjoin_np(synth_intel_ptwrite *event);
void tmon_pthread_cond_init(synth_intel_ptwrite *event);
// #8-#10 are missing.
void tmon_pthread_cond_signal(synth_intel_ptwrite *event);
void tmon_pthread_cond_broadcast(synth_intel_ptwrite *event);
void tmon_pthread_cond_destroy(synth_intel_ptwrite *event);
void tmon_pthread_mutex_init(synth_intel_ptwrite *event);
void tmon_pthread_mutex_destroy(synth_intel_ptwrite *event);
void tmon_pthread_mutex_trylock(synth_intel_ptwrite *event);
void tmon_pthread_mutex_timedlock(synth_intel_ptwrite *event);
void tmon_pthread_spin_init(synth_intel_ptwrite *event);
void tmon_pthread_spin_destroy(synth_intel_ptwrite *event);
void tmon_pthread_spin_lock(synth_intel_ptwrite *event);
void tmon_pthread_spin_trylock(synth_intel_ptwrite *event);
void tmon_pthread_spin_unlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_init(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_destroy(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_rdlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_tryrdlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_timedrdlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_wrlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_trywrlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_timedwrlock(synth_intel_ptwrite *event);
void tmon_pthread_rwlock_unlock(synth_intel_ptwrite *event);
void tmon_pthread_barrier_init(synth_intel_ptwrite *event);
void tmon_pthread_barrier_destroy(synth_intel_ptwrite *event);
void tmon_pthread_barrier_wait(synth_intel_ptwrite *event);
// #35 is missing
void tmon_pthread_sigmask(synth_intel_ptwrite *event);
EventFunction tmon_pthread_kill_ptr;

// Common Interceptors --------------------------------------
// #1 and #2 are missing.
void tmon_pthread_mutex_lock(synth_intel_ptwrite *event);
void tmon_pthread_mutex_unlock(synth_intel_ptwrite *event);
void tmon___pthread_mutex_lock(synth_intel_ptwrite *event);
void tmon___pthread_mutex_unlock(synth_intel_ptwrite *event);

// Similar to Tsan, to be defined by user.
EventFunction tmon_volatile_read1;
EventFunction tmon_volatile_read2;
EventFunction tmon_volatile_read4;
EventFunction tmon_volatile_read8;
EventFunction tmon_volatile_read16;
EventFunction tmon_volatile_write1;
EventFunction tmon_volatile_write2;
EventFunction tmon_volatile_write4;
EventFunction tmon_volatile_write8;
EventFunction tmon_volatile_write16;
EventFunction tmon_unaligned_volatile_read1;
EventFunction tmon_unaligned_volatile_read2;
EventFunction tmon_unaligned_volatile_read4;
EventFunction tmon_unaligned_volatile_read8;
EventFunction tmon_unaligned_volatile_read16;
EventFunction tmon_unaligned_volatile_write1;
EventFunction tmon_unaligned_volatile_write2;
EventFunction tmon_unaligned_volatile_write4;
EventFunction tmon_unaligned_volatile_write8;
EventFunction tmon_unaligned_volatile_write16;
EventFunction tmon_read_write1;
EventFunction tmon_read_write2;
EventFunction tmon_read_write4;
EventFunction tmon_read_write8;
EventFunction tmon_read_write16;
EventFunction tmon_unaligned_read_write1;
EventFunction tmon_unaligned_read_write2;
EventFunction tmon_unaligned_read_write4;
EventFunction tmon_unaligned_read_write8;
EventFunction tmon_unaligned_read_write16;

#endif // TMON_EH_H