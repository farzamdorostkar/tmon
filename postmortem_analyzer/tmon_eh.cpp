/***************************************************************
 * Author: Farzam Dorostkar
 * 
 * This file is part of a post-mortem api designed to access 
 * TSan runtime library race detection logic.
 **************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "tsan_rtl.h"
#include "sanitizer_internal_defs.h"
#include <map>
#include "common.h"
#include <stdbool.h>
#include "tsan_interface.h"
#include "sanitizer_common/sanitizer_ptrauth.h"
#include "tsan_interceptors.h"
#include <errno.h>
#include "sanitizer_errno_codes.h"
#include "tmon_defs.h"

// <OsTid, ThreadState*>
std::map<u32t, __tsan::ThreadState*> TState;

// <OsTid, pc>
std::map<u32t, __tsan::uptr> TopPC;

// <pthread_t, __tsan::Tid>
std::map<u64t, __tsan::Tid> PHandle_TSanTid;

// <pthread_t, ThreadState*>
std::map<u64t, __tsan::ThreadState*> PHandle_TState;

// <OsTid, pthread_t>
std::map<u32t, u64t> OSTid_PHandle;

// <pthread_t, OsTid>
std::map<u64t, u32t> PHandle_OSTid;

bool is_first = true;

__tsan::ThreadState *get_threadstate(u32t tid, bool new_thread)
{
  auto it = TState.find(tid);

  if (it == TState.end()) {
    // tid does not exist.
    if (is_first) {
      is_first = false;
      __tsan::ThreadState *main_thr = (__tsan::ThreadState *)calloc(1, sizeof(*main_thr));
      TState.insert({tid, main_thr});
      __tsan::CurThread = main_thr;
      Initialize(main_thr);
      return main_thr;
    } else if (new_thread) {
      __tsan::ThreadState *new_thr = (__tsan::ThreadState *)calloc(1, sizeof(*new_thr));
      TState.insert({tid, new_thr});
      return new_thr;
    } else {
      printf("ERROR: unknown new tid!\n");
      exit(1);
    }
  }
  
  return TState.at(tid);
}

void update_toppc(u32t tid, __tsan::uptr pc)
{
  auto it = TopPC.find(tid);

  if (it == TopPC.end())
    TopPC.insert({tid, pc});
  else
    it->second = pc;
}

#define SCOPED_INTERCEPTOR_RAW_PM(func, ...)            \
  __tsan::ThreadState *thr = __tsan::cur_thread_init(); \
  __tsan::ScopedInterceptor si(thr, #func, TopPC[ostid]); \
  UNUSED const uptr pc = TopPC[ostid];

#  define SANITIZER_LINUX 1

#define SCOPED_TSAN_INTERCEPTOR_PM(func, ...)   \
  SCOPED_INTERCEPTOR_RAW_PM(func, __VA_ARGS__);

struct TsanInterceptorContext {
  __tsan::ThreadState *thr;
  const uptr pc;
};

#define COMMON_INTERCEPTOR_ENTER_PM(ctx, func, ...) \
  SCOPED_TSAN_INTERCEPTOR_PM(func, __VA_ARGS__); \
  TsanInterceptorContext _ctx = {thr, pc};       \
  ctx = (void *)&_ctx;                           \
  (void)ctx;

//same as tsan
#define COMMON_INTERCEPTOR_MUTEX_PRE_LOCK(ctx, m) \
  MutexPreLock(((TsanInterceptorContext *)ctx)->thr, \
            ((TsanInterceptorContext *)ctx)->pc, (uptr)m)

//same as tsan
#define COMMON_INTERCEPTOR_MUTEX_POST_LOCK(ctx, m) \
  MutexPostLock(((TsanInterceptorContext *)ctx)->thr, \
            ((TsanInterceptorContext *)ctx)->pc, (uptr)m)

// same as TSan (used by common interceptors)
#define COMMON_INTERCEPTOR_MUTEX_REPAIR(ctx, m) \
  __tsan::MutexRepair(((TsanInterceptorContext *)ctx)->thr, \
            ((TsanInterceptorContext *)ctx)->pc, (uptr)m)

// same as TSan (used by common interceptors)
#define COMMON_INTERCEPTOR_MUTEX_INVALID(ctx, m) \
  __tsan::MutexInvalidAccess(((TsanInterceptorContext *)ctx)->thr, \
                     ((TsanInterceptorContext *)ctx)->pc, (uptr)m)

//same as tsan (used by common interceptors)
#define COMMON_INTERCEPTOR_MUTEX_UNLOCK(ctx, m) \
  __tsan::MutexUnlock(((TsanInterceptorContext *)ctx)->thr, \
            ((TsanInterceptorContext *)ctx)->pc, (uptr)m)

static const uint64_t SixLSBMask = 0x0000ffffffffffffULL;
static const uint64_t SeventhByteMask = 0x00ff000000000000ULL;
static const uint64_t SHL = 48;

// -------------------------------------------------------------------
// Event Functions ---------------------------------------------------

void tmon_func_entry(synth_intel_ptwrite *event)
{
	uint64_t pc = (event->payload) & 0xffffffffffff;
	update_toppc(event->tid, pc);
	__tsan_func_entry((void *)pc);
}

void tmon_function_exit(synth_intel_ptwrite *event)
{
  __tsan_func_exit();
}

void tmon_read1(synth_intel_ptwrite *event)
{
  u64t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 1, __tsan::kAccessRead);
}

void tmon_read2(synth_intel_ptwrite *event)
{
  u64t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 2, __tsan::kAccessRead);
}

void tmon_read4(synth_intel_ptwrite *event)
{
  u64t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 4, __tsan::kAccessRead);
}

void tmon_read8(synth_intel_ptwrite *event)
{
  u64t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessRead);
}

namespace __tsan
{
  // MemoryAccess16 is not declared in tsan_rtl.h.
  void MemoryAccess16(ThreadState* thr, uptr pc, uptr addr, AccessType typ);
}

void tmon_read16(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess16(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, __tsan::kAccessRead);
}

void tmon_write1(synth_intel_ptwrite *event)
{
	u64t addr = (event->payload) & 0xffffffffffff;
	MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 1, __tsan::kAccessWrite);
}

void tmon_write2(synth_intel_ptwrite *event)
{
	u64t addr = (event->payload) & 0xffffffffffff;
	MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 2, __tsan::kAccessWrite);
}

void tmon_write4(synth_intel_ptwrite *event)
{
	u64t addr = (event->payload) & 0xffffffffffff;
	MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 4, __tsan::kAccessWrite);
}

void tmon_write8(synth_intel_ptwrite *event)
{
	u64t addr = (event->payload) & 0xffffffffffff;
	MemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessWrite);
}

void tmon_write16(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  MemoryAccess16(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, __tsan::kAccessWrite);
}

void tmon_unaligned_read1(synth_intel_ptwrite *event)
{
  // Note: TSan's instrumentation pass declares __tsan_unaligned_read1 but its runtime has no 
  // implementation for this function (since a 1-byte access is always aligned). We define 
  // PTW_UNALIGNED_READ1 and the noop function tmon_unaligned_read1 just to maintain consistency with TSan.
}

void tmon_unaligned_read2(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 2, __tsan::kAccessRead);
}

void tmon_unaligned_read4(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 4, __tsan::kAccessRead);
}

void tmon_unaligned_read8(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessRead);
}

void tmon_unaligned_read16(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessRead);
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr + 8, 8, __tsan::kAccessRead);
}

void tmon_unaligned_write1(synth_intel_ptwrite *event)
{
  // Note: TSan's instrumentation pass declares __tsan_unaligned_write1 but its runtime has no 
  // implementation for this function (since a 1-byte access is always aligned). We define 
  // PTW_UNALIGNED_WRITE1 and the noop function tmon_unaligned_write1 just to maintain consistency with TSan.
}

void tmon_unaligned_write2(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 2, __tsan::kAccessWrite);
}

void tmon_unaligned_write4(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 4, __tsan::kAccessWrite);
}

void tmon_unaligned_write8(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessWrite);
}

void tmon_unaligned_write16(synth_intel_ptwrite *event)
{
  uint64_t addr = (event->payload) & 0xffffffffffff;
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr, 8, __tsan::kAccessWrite);
  __tsan::UnalignedMemoryAccess(__tsan::cur_thread(), TopPC[event->tid], (uptr)addr + 8, 8, __tsan::kAccessWrite);
}

// NOTE: TSan runtime currently does not support the different instrumentation of volatile and compound accesses, neither does TMon.

void tmon_thread_start(synth_intel_ptwrite *event)
{
  uint32_t ostid = event->tid;
  uint64_t handle = (event->payload) & SixLSBMask;

  auto it = PHandle_TSanTid.find(handle);
  if (it == PHandle_TSanTid.end()) {
    printf("ERROR: unknown pthread handle");
    exit(1);
  }
  __tsan::Tid atid = it->second;

  PHandle_TState.insert({handle, __tsan::cur_thread()});
  PHandle_OSTid.insert({handle, ostid});
  OSTid_PHandle.insert({ostid, handle});

  {
    __tsan::ThreadState *thr = __tsan::cur_thread_init();
    // Thread-local state is not initialized yet.
    __tsan::ScopedIgnoreInterceptors ignore;

    //TODO: omitted #if
  #if !SANITIZER_APPLE && !SANITIZER_NETBSD && !SANITIZER_FREEBSD
    ThreadIgnoreBegin(thr, 0);
    /*if (pthread_setspecific(interceptor_ctx()->finalize_key,
                            (void *)__tsan::GetPthreadDestructorIterations())) {
      printf("ThreadSanitizer: failed to set thread key\n");
      //Die();
    }*/
    ThreadIgnoreEnd(thr);
  #endif

    __tsan::Processor *proc = __tsan::ProcCreate();
    ProcWire(proc, thr);
    __tsan::ThreadStart(thr, atid, ostid, __sanitizer::ThreadType::Regular);
    printf("return from ThreadStart - thr->tid:%d thr->clock[0]:%hu\n", thr->tid, thr->clock.Get(static_cast<__tsan::Sid>(0))); 
  }
}

// TSan Specific Interceptors ----------------------------------

//
void tmon_pthread_create(synth_intel_ptwrite *event)
{
  uint32_t ostid = event->tid;
  uint64_t handle = (event->payload) & SixLSBMask;

  // DIFF: pc is the latest return address of tid not interceptor_pthread_create
  SCOPED_INTERCEPTOR_RAW_PM(pthread_create);

  if (__tsan::ctx->after_multithreaded_fork) {
    if (__tsan::flags()->die_after_fork) {
      printf("ThreadSanitizer: starting new threads after multi-threaded "
          "fork is not supported. Dying (set die_after_fork=0 to override)\n");
      exit(1);
    } else {
      printf( "ThreadSanitizer: starting new threads after multi-threaded "
              "fork is not supported (pid %d). Continuing because of "
              "die_after_fork=0, but you are on your own\n",
              ostid);
    }
  }

  __tsan::Tid atid = __tsan::kMainTid;
  atid = __tsan::ThreadCreate(thr, pc, handle, __tsan::IsStateDetached(0));
  //printf("atid: %d\n", atid);
  CHECK_NE(atid, __tsan::kMainTid);

  PHandle_TSanTid.insert({handle, atid});
}

// For now just used by tmon_pthread_join 
void DestroyThreadState_PM(__tsan::ThreadState *thr) {
  //ThreadState *thr = cur_thread();
  __tsan::Processor *proc = thr->proc();
  ThreadFinish(thr);
  ProcUnwire(proc, thr);
  ProcDestroy(proc);
  //DTLS_Destroy();
  __tsan::cur_thread_finalize();
}

//
void tmon_pthread_join(synth_intel_ptwrite *event)
{
  uint64_t th = (event->payload) & SixLSBMask;
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint32_t ostid = event->tid;
  uint32_t pid = event->pid;

  // [TMon] Lazy DestroyThreadState for thread th.
  //DestroyThreadState_PM(PHandle_TState[th]);
  // ---------------------------------------

  SCOPED_INTERCEPTOR_RAW_PM(pthread_join);
  __tsan::Tid tid = ThreadConsumeTid(thr, pc, (uptr)th);
  ThreadIgnoreBegin(thr, pc);
  ThreadIgnoreEnd(thr);
  if (res == 0)
    ThreadJoin(thr, pc, tid);
  
  // [TMon] Analyze errno.
  if (res == ESRCH) {
    printf("WARNING: ThreadMonitor: unknown pthread_t ID (tid=%d)\n", ostid);
    printf("  No thread with the ID %ld could be found.\n", th);
  } else if (res == EINVAL) {
    printf("WARNING: ThreadMonitor: invalid pthread_t ID (tid=%d)\n", ostid);
    // Thread is not a joinable thread or another thread is already waiting to join with this thread.
    auto it = PHandle_TSanTid.find(th);
    // If pthread_t does not exist then it must be the main thread.
    __tsan::Tid tsantid = (it == PHandle_TSanTid.end()) ? 0 : it->second;
    __tsan::ThreadContextBase *tctx = __tsan::ctx->thread_registry.GetThreadContextBase(tsantid);
    if (tctx->detached)
      printf("Thread %ld is not a joinable thread\n", th);
    else
      printf("Another thread is already waiting to join with thread %ld\n", th);
  } else if (res == EDEADLK) {
    printf("WARNING: ThreadMonitor: deadlock (tid=%d)\n", ostid);
    // Two threads tried to join with each other or a thread tried to join with itself.
    auto it_phandle = OSTid_PHandle.find(ostid);
    if (it_phandle == OSTid_PHandle.end()) {
      auto it_ostid = PHandle_OSTid.find(th);
      if (it_ostid == PHandle_OSTid.end())
        printf("Thread %ld tried to join with itself\n", th);
      else
        printf("Two threads tried to join with each other\n");
    } else if (it_phandle->second == th) {
      printf("Thread %ld tried to join with itself\n", th);
    } else
      printf("Two threads tried to join with each other\n");
  }
  // TODO: Maybe ThreadIgnoreBegin and ThreadIgnoreEnd can be removed.
}

//
void tmon_pthread_detach(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t th = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_INTERCEPTOR_RAW_PM(pthread_detach);
  __tsan::Tid tid = ThreadConsumeTid(thr, pc, (uptr)th);
  if (res == 0) {
    ThreadDetach(thr, pc, tid);
  }
}

//
void tmon_pthread_exit(synth_intel_ptwrite *event)
{
  uint32_t ostid = event->tid;

  {
    SCOPED_INTERCEPTOR_RAW_PM(pthread_exit, retval);
/*#if !SANITIZER_APPLE && !SANITIZER_ANDROID
    CHECK_EQ(thr, &cur_thread_placeholder);
#endif*/
  }
  // TODO: Double check if commented lines can be removed.
}

#if SANITIZER_LINUX
//
void tmon_pthread_tryjoin_np(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t th = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_INTERCEPTOR_RAW_PM(pthread_tryjoin_np);
  __tsan::Tid tid = ThreadConsumeTid(thr, pc, (uptr)th);
  ThreadIgnoreBegin(thr, pc);
  // Removed call to REAL(pthread_tryjoin_np).
  ThreadIgnoreEnd(thr);
  if (res == 0)
    ThreadJoin(thr, pc, tid);
  else
    ThreadNotJoined(thr, pc, tid, (uptr)th);
  // TODO: Maybe ThreadIgnoreBegin and ThreadIgnoreEnd can be removed.
}

//
void tmon_pthread_timedjoin_np(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t th = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_INTERCEPTOR_RAW_PM(pthread_timedjoin_np);
  __tsan::Tid tid = ThreadConsumeTid(thr, pc, (uptr)th);
  ThreadIgnoreBegin(thr, pc);
  // Removed call to BLOCK_REAL(pthread_timedjoin_np)
  ThreadIgnoreEnd(thr);
  if (res == 0)
    ThreadJoin(thr, pc, tid);
  else
    ThreadNotJoined(thr, pc, tid, (uptr)th);
  // TODO: Maybe ThreadIgnoreBegin and ThreadIgnoreEnd can be removed.
}
#endif

//
void tmon_pthread_cond_init(synth_intel_ptwrite *event)
{
  uint64_t c = (event->payload) & SixLSBMask; // c is a pthread_cond_t*
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_cond_init);
  MemoryAccessRange(thr, pc, (uptr)c, sizeof(uptr), true);
}

//
void tmon_pthread_cond_signal(synth_intel_ptwrite *event)
{
  uint64_t c = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_cond_signal);
  MemoryAccessRange(thr, pc, (uptr)c, sizeof(uptr), false);
}

//
void tmon_pthread_cond_broadcast(synth_intel_ptwrite *event)
{
  uint64_t c = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_cond_broadcast);
  MemoryAccessRange(thr, pc, (uptr)c, sizeof(uptr), false);
}

// #13
void tmon_pthread_cond_destroy(synth_intel_ptwrite *event)
{
  uint64_t c = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  //void *cond = init_cond(c);
  SCOPED_TSAN_INTERCEPTOR_PM(pthread_cond_destroy);
  MemoryAccessRange(thr, pc, (uptr)c, sizeof(uptr), true);
  /*if (common_flags()->legacy_pthread_cond) {
    // Free our aux cond and zero the pointer to not leave dangling pointers.
    WRAP(free)(cond);
    atomic_store((atomic_uintptr_t*)c, 0, memory_order_relaxed);
  }*/
  //TODO: Double check if commented lines can be removed.
}

// #14
void tmon_pthread_mutex_init(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask; // m is a pthread_mutex_t*
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_mutex_init);
  if (res == 0) {
    __sanitizer::u32 flagz = 0;
    // TODO: Assumed that attr is NULL.
    MutexCreate(thr, pc, (uptr)m, flagz);
  }
}

// #15
void tmon_pthread_mutex_destroy(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_mutex_destroy);
  if (res == 0 || res == errno_EBUSY) {
    MutexDestroy(thr, pc, (uptr)m);
  }
}

// #16
void tmon_pthread_mutex_trylock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_mutex_trylock);
  if (res == __sanitizer::errno_EOWNERDEAD)
    MutexRepair(thr, pc, (uptr)m);
  if (res == 0 || res == __sanitizer::errno_EOWNERDEAD)
    MutexPostLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
}

// #17
void tmon_pthread_mutex_timedlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_mutex_timedlock);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
  }
}

// #18
void tmon_pthread_spin_init(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  // m is a pthread_spinlock_t*
  SCOPED_TSAN_INTERCEPTOR_PM(pthread_spin_init);
  if (res == 0) {
    MutexCreate(thr, pc, (uptr)m);
  }
}

// #19
void tmon_pthread_spin_destroy(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_spin_destroy);
  if (res == 0) {
    MutexDestroy(thr, pc, (uptr)m);
  }
}

// #20
void tmon_pthread_spin_lock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_spin_lock);
  MutexPreLock(thr, pc, (uptr)m);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m);
  }
}

// #21
void tmon_pthread_spin_trylock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_spin_trylock);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
  }
}

// #22
void tmon_pthread_spin_unlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_spin_unlock);
  MutexUnlock(thr, pc, (uptr)m);
}

// #23
void tmon_pthread_rwlock_init(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask; // m is a pthread_rwlock_t*
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_init);
  if (res == 0) {
    MutexCreate(thr, pc, (uptr)m);
  }
}

// #24
void tmon_pthread_rwlock_destroy(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_destroy);
  if (res == 0) {
    MutexDestroy(thr, pc, (uptr)m);
  }
}

// #25
void tmon_pthread_rwlock_rdlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_rdlock);
  MutexPreReadLock(thr, pc, (uptr)m);
  if (res == 0) {
    MutexPostReadLock(thr, pc, (uptr)m);
  }
}

// #26
void tmon_pthread_rwlock_tryrdlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_tryrdlock);
  if (res == 0) {
    MutexPostReadLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
  }
}

// #27
void tmon_pthread_rwlock_timedrdlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_timedrdlock);
  if (res == 0) {
    MutexPostReadLock(thr, pc, (uptr)m);
  }
}

// #28
void tmon_pthread_rwlock_wrlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_wrlock);
  MutexPreLock(thr, pc, (uptr)m);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m);
  }
}

// #29
void tmon_pthread_rwlock_trywrlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_trywrlock);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
  }
}

// #30
void tmon_pthread_rwlock_timedwrlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_timedwrlock);
  if (res == 0) {
    MutexPostLock(thr, pc, (uptr)m, __tsan::MutexFlagTryLock);
  }
}

// #31
void tmon_pthread_rwlock_unlock(synth_intel_ptwrite *event)
{
  uint64_t m = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_rwlock_unlock);
  MutexReadOrWriteUnlock(thr, pc, (uptr)m);
}

// #32
void tmon_pthread_barrier_init(synth_intel_ptwrite *event)
{
  uint64_t b = (event->payload) & SixLSBMask; // b is a pthread_barrier_t*
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_barrier_init);
  MemoryAccess(thr, pc, (uptr)b, 1, __tsan::kAccessWrite);
}

// #33
void tmon_pthread_barrier_destroy(synth_intel_ptwrite *event)
{
  uint64_t b = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_barrier_destroy);
  MemoryAccess(thr, pc, (uptr)b, 1, __tsan::kAccessWrite);
}

const int PTHREAD_BARRIER_SERIAL_THREAD = -1;

// #34
void tmon_pthread_barrier_wait(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  uint64_t b = (event->payload) & SixLSBMask;
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_barrier_wait);
  Release(thr, pc, (uptr)b);
  MemoryAccess(thr, pc, (uptr)b, 1, __tsan::kAccessRead);
  // TODO: Check if both MemoryAccess calls are required.
  MemoryAccess(thr, pc, (uptr)b, 1, __tsan::kAccessRead);
  if (res == 0 || res == PTHREAD_BARRIER_SERIAL_THREAD) {
    Acquire(thr, pc, (uptr)b);
  }
}

// #35
// TODO: pthread_once

// #36
void tmon_pthread_sigmask(synth_intel_ptwrite *event)
{
  uint32_t ostid = event->tid;

  SCOPED_TSAN_INTERCEPTOR_PM(pthread_sigmask);
}

// Common Interceptors -----------------------------------------

// #3
void tmon_pthread_mutex_lock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  void *m = (void *)((event->payload) & SixLSBMask);
  uint32_t ostid = event->tid;

  void *ctx;
  COMMON_INTERCEPTOR_ENTER_PM(ctx, pthread_mutex_lock, m);
  COMMON_INTERCEPTOR_MUTEX_PRE_LOCK(ctx, m);
  if (res == __sanitizer::errno_EOWNERDEAD)
    COMMON_INTERCEPTOR_MUTEX_REPAIR(ctx, m);
  if (res == 0 || res == __sanitizer::errno_EOWNERDEAD)
    COMMON_INTERCEPTOR_MUTEX_POST_LOCK(ctx, m);
  if (res == errno_EINVAL)
    COMMON_INTERCEPTOR_MUTEX_INVALID(ctx, m);

  COMMON_INTERCEPTOR_MUTEX_POST_LOCK(ctx, m);
}

// #4
void tmon_pthread_mutex_unlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  void *m = (void *)((event->payload) & SixLSBMask);
  uint32_t ostid = event->tid;

  void *ctx;
  COMMON_INTERCEPTOR_ENTER_PM(ctx, pthread_mutex_unlock, m);
  COMMON_INTERCEPTOR_MUTEX_UNLOCK(ctx, m);
  if (res == errno_EINVAL)
     COMMON_INTERCEPTOR_MUTEX_INVALID(ctx, m);
}

// #5
void tmon___pthread_mutex_lock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  void *m = (void *)((event->payload) & SixLSBMask);
  uint32_t ostid = event->tid;

  void *ctx;
  COMMON_INTERCEPTOR_ENTER_PM(ctx, __pthread_mutex_lock, m);
  COMMON_INTERCEPTOR_MUTEX_PRE_LOCK(ctx, m);
  if (res == __sanitizer::errno_EOWNERDEAD)
    COMMON_INTERCEPTOR_MUTEX_REPAIR(ctx, m);
  if (res == 0 || res == __sanitizer::errno_EOWNERDEAD)
    COMMON_INTERCEPTOR_MUTEX_POST_LOCK(ctx, m);
  if (res == errno_EINVAL)
    COMMON_INTERCEPTOR_MUTEX_INVALID(ctx, m);
}

// #6
void tmon___pthread_mutex_unlock(synth_intel_ptwrite *event)
{
  uint64_t res = ((event->payload) & SeventhByteMask) >> SHL;
  void *m = (void *)((event->payload) & SixLSBMask);
  uint32_t ostid = event->tid;

  void *ctx;
  COMMON_INTERCEPTOR_ENTER_PM(ctx, __pthread_mutex_unlock, m);
  COMMON_INTERCEPTOR_MUTEX_UNLOCK(ctx, m);
  if (res == errno_EINVAL)
    COMMON_INTERCEPTOR_MUTEX_INVALID(ctx, m);
}

const int kSigCount = 65;

__tsan::ThreadSignalContext *SigCtx(__tsan::ThreadState *thr);

struct ucontext_t {
  // The size is determined by looking at sizeof of real ucontext_t on linux.
  u64t opaque[936 / sizeof(u64t) + 1];
};

namespace __tsan {

struct SignalDesc {
  bool armed;
  __sanitizer_siginfo siginfo;
  ucontext_t ctx;
};

struct ThreadSignalContext {
  int int_signal_send;
  atomic_uintptr_t in_blocking_func;
  SignalDesc pending_signals[kSigCount];
  // emptyset and oldset are too big for stack.
  __sanitizer_sigset_t emptyset;
  __sanitizer_sigset_t oldset;
};
}

void tmon_pthread_kill(bool self, int sig, uint32_t ostid)
{
  SCOPED_TSAN_INTERCEPTOR_PM(pthread_kill, tid, sig);
  __tsan::ThreadSignalContext *sctx = SigCtx(thr);
  CHECK_NE(sctx, 0);
  int prev = sctx->int_signal_send;
  if (self)
    sctx->int_signal_send = sig;
  if (self) {
    CHECK_EQ(sctx->int_signal_send, sig);
    sctx->int_signal_send = prev;
  }
}