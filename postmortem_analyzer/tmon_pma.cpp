
#include "common.h"
#include "tmon_eh.h"

#include <ptwrite_instrumentation.h>
#include <tsan_rtl.h>

__tsan::ThreadState *get_threadstate(u32t tid, bool new_thread);

const uint64_t event_mask = (uint64_t)0xff << LSH;

EventFunction eventHandlers[] = {
	tmon_func_entry,
	tmon_function_exit,
	tmon_ignore_thread_begin,
	tmon_ignore_thread_end,
	tmon_read1,
	tmon_read2,
	tmon_read4,
	tmon_read8,
	tmon_read16,
	tmon_write1,
	tmon_write2,
	tmon_write4,
	tmon_write8,
	tmon_write16,
	tmon_unaligned_read1,
	tmon_unaligned_read2,
	tmon_unaligned_read4,
	tmon_unaligned_read8,
	tmon_unaligned_read16,
	tmon_unaligned_write1,
	tmon_unaligned_write2,
	tmon_unaligned_write4,
	tmon_unaligned_write8,
	tmon_unaligned_write16,
	tmon_volatile_read1,
	tmon_volatile_read2,
	tmon_volatile_read4,
	tmon_volatile_read8,
	tmon_volatile_read16,
	tmon_volatile_write1,
	tmon_volatile_write2,
	tmon_volatile_write4,
	tmon_volatile_write8,
	tmon_volatile_write16,
	tmon_unaligned_volatile_read1,
	tmon_unaligned_volatile_read2,
	tmon_unaligned_volatile_read4,
	tmon_unaligned_volatile_read8,
	tmon_unaligned_volatile_read16,
	tmon_unaligned_volatile_write1,
	tmon_unaligned_volatile_write2,
	tmon_unaligned_volatile_write4,
	tmon_unaligned_volatile_write8,
	tmon_unaligned_volatile_write16,
	tmon_read_write1,
	tmon_read_write2,
	tmon_read_write4,
	tmon_read_write8,
	tmon_read_write16,
	tmon_unaligned_read_write1,
	tmon_unaligned_read_write2,
	tmon_unaligned_read_write4,
	tmon_unaligned_read_write8,
	tmon_unaligned_read_write16,
	tmon_vptr_read,
	tmon_vptr_write,
	tmon_atomic8_load,
	tmon_atomic16_load,
	tmon_atomic32_load,
	tmon_atomic64_load,
	tmon_atomic128_load,
	tmon_atomic8_store,
	tmon_atomic16_store,
	tmon_atomic32_store,
	tmon_atomic64_store,
	tmon_atomic128_store,
  tmon_atomic8_rmw,
	tmon_atomic16_rmw,
	tmon_atomic32_rmw,
	tmon_atomic64_rmw,
	tmon_atomic128_rmw,
  tmon_atomic8_cmpxchg,
	tmon_atomic16_cmpxchg,
	tmon_atomic32_cmpxchg,
	tmon_atomic64_cmpxchg,
	tmon_atomic128_cmpxchg,
	// TSan Specific Interceptors
	tmon_thread_start,
	tmon_pthread_create,
	tmon_pthread_join,
	tmon_pthread_detach,
	tmon_pthread_exit,
	tmon_pthread_tryjoin_np,
	tmon_pthread_timedjoin_np,
	tmon_pthread_cond_init,
	tmon_pthread_cond_signal,
	tmon_pthread_cond_broadcast,
	tmon_pthread_cond_destroy,
	tmon_pthread_mutex_init,
	tmon_pthread_mutex_destroy,
	tmon_pthread_mutex_trylock,
	tmon_pthread_mutex_timedlock,
	tmon_pthread_spin_init,
	tmon_pthread_spin_destroy,
	tmon_pthread_spin_lock,
	tmon_pthread_spin_trylock,
	tmon_pthread_spin_unlock,
	tmon_pthread_rwlock_init,
	tmon_pthread_rwlock_destroy,
	tmon_pthread_rwlock_rdlock,
	tmon_pthread_rwlock_tryrdlock,
	tmon_pthread_rwlock_timedrdlock,
	tmon_pthread_rwlock_wrlock,
	tmon_pthread_rwlock_trywrlock,
	tmon_pthread_rwlock_timedwrlock,
	tmon_pthread_rwlock_unlock,
	tmon_pthread_barrier_init,
	tmon_pthread_barrier_destroy,
	tmon_pthread_barrier_wait,
	tmon_pthread_sigmask,
	tmon_pthread_kill_ptr,
	// Common Interceptors
	tmon_pthread_mutex_lock,
	tmon_pthread_mutex_unlock,
	tmon___pthread_mutex_lock,
	tmon___pthread_mutex_unlock,
};

void tmon_pass_event(struct synth_intel_ptwrite *event) 
{
	uint64_t event_type = ((event->payload) & event_mask) >> LSH; //TODO: replace 56 with LSH

	__tsan::ThreadState *thr = get_threadstate(event->tid, event_type == PTW_THREAD_START);
	__tsan::CurThread = thr;

	eventHandlers[event_type - 1](event);
}

int main(int argc, const char **argv)
{

  tmon_perf(argc, argv);

  return 0;
}