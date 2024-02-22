cmd_trace/beauty/ioctl.o := gcc -Wp,-MD,trace/beauty/.ioctl.o.d -Wp,-MT,trace/beauty/ioctl.o -Wbad-function-cast -Wdeclaration-after-statement -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-prototypes -Wno-system-headers -Wold-style-definition -Wpacked -Wredundant-decls -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wundef -Wwrite-strings -Wformat -Wno-type-limits -Wstrict-aliasing=3 -Wshadow -DHAVE_SYSCALL_TABLE_SUPPORT -DHAVE_ARCH_X86_64_SUPPORT -Iarch/x86/include/generated -DHAVE_PERF_REGS_SUPPORT -DHAVE_ARCH_REGS_QUERY_REGISTER_OFFSET -Werror -DNDEBUG=1 -O6 -fno-omit-frame-pointer -Wall -Wextra -std=gnu11 -fstack-protector-all -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/arch/x86/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf -DHAVE_PTHREAD_ATTR_SETAFFINITY_NP -DHAVE_PTHREAD_BARRIER -DHAVE_EVENTFD_SUPPORT -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_GETTID -DHAVE_FILE_HANDLE -DHAVE_DWARF_GETLOCATIONS_SUPPORT -DHAVE_DWARF_CFI_SUPPORT -DHAVE_AIO_SUPPORT -DHAVE_SCANDIRAT_SUPPORT -DHAVE_SCHED_GETCPU_SUPPORT -DHAVE_SETNS_SUPPORT -DHAVE_ZLIB_SUPPORT -DHAVE_LIBELF_SUPPORT -DHAVE_ELF_GETPHDRNUM_SUPPORT -DHAVE_GELF_GETNOTE_SUPPORT -DHAVE_ELF_GETSHDRSTRNDX_SUPPORT -DHAVE_DWARF_SUPPORT -DHAVE_LIBBPF_SUPPORT -DHAVE_SDT_EVENT -DHAVE_JITDUMP -DHAVE_BPF_SKEL -DHAVE_DWARF_UNWIND_SUPPORT -DNO_LIBUNWIND_DEBUG_FRAME -DHAVE_LIBUNWIND_SUPPORT -DHAVE_LIBCRYPTO_SUPPORT -DHAVE_SLANG_SUPPORT -DHAVE_LIBPERL_SUPPORT -DHAVE_TIMERFD_SUPPORT -DHAVE_LIBPYTHON_SUPPORT -DHAVE_CXA_DEMANGLE_SUPPORT -DHAVE_LZMA_SUPPORT -DHAVE_ZSTD_SUPPORT -DHAVE_LIBCAP_SUPPORT -DHAVE_BACKTRACE_SUPPORT -DHAVE_LIBNUMA_SUPPORT -DHAVE_KVM_STAT_SUPPORT -DDISASM_FOUR_ARGS_SIGNATURE -DHAVE_PERF_READ_VDSO32 -DHAVE_PERF_READ_VDSOX32 -DHAVE_LIBBABELTRACE_SUPPORT -DHAVE_AUXTRACE_SUPPORT -DHAVE_JVMTI_CMLR -DHAVE_LIBPFM -DHAVE_LIBTRACEEVENT -DLIBTRACEEVENT_VERSION=65793 -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/tmon_perf/libapi/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/tmon_perf/libbpf/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/tmon_perf/libsubcmd/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/tmon_perf/libsymbol/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/tmon_perf/libperf/include -D"BUILD_STR(s)=$(pound)s" -c -o trace/beauty/ioctl.o trace/beauty/ioctl.c

source_trace/beauty/ioctl.o := trace/beauty/ioctl.c

deps_trace/beauty/ioctl.o := \
  /usr/include/stdc-predef.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/beauty.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/kernel.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdarg.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stddef.h \
  /usr/include/assert.h \
  /usr/include/features.h \
  /usr/include/features-time64.h \
  /usr/include/x86_64-linux-gnu/bits/wordsize.h \
  /usr/include/x86_64-linux-gnu/bits/timesize.h \
  /usr/include/x86_64-linux-gnu/sys/cdefs.h \
  /usr/include/x86_64-linux-gnu/bits/long-double.h \
  /usr/include/x86_64-linux-gnu/gnu/stubs.h \
  /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/build_bug.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler_types.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler-gcc.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/types.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdbool.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdint.h \
  /usr/include/stdint.h \
  /usr/include/x86_64-linux-gnu/bits/libc-header-start.h \
  /usr/include/x86_64-linux-gnu/bits/types.h \
  /usr/include/x86_64-linux-gnu/bits/typesizes.h \
  /usr/include/x86_64-linux-gnu/bits/time64.h \
  /usr/include/x86_64-linux-gnu/bits/wchar.h \
  /usr/include/x86_64-linux-gnu/bits/stdint-intn.h \
  /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h \
  /usr/include/x86_64-linux-gnu/asm/types.h \
  /usr/include/asm-generic/types.h \
  /usr/include/asm-generic/int-ll64.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi/asm/bitsperlong.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/asm-generic/bitsperlong.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi/asm-generic/bitsperlong.h \
  /usr/include/x86_64-linux-gnu/asm/posix_types.h \
  /usr/include/x86_64-linux-gnu/asm/posix_types_64.h \
  /usr/include/asm-generic/posix_types.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/math.h \
  /usr/include/endian.h \
  /usr/include/x86_64-linux-gnu/bits/endian.h \
  /usr/include/x86_64-linux-gnu/bits/endianness.h \
  /usr/include/x86_64-linux-gnu/bits/byteswap.h \
  /usr/include/x86_64-linux-gnu/bits/uintn-identity.h \
  /usr/include/byteswap.h \
  /usr/include/x86_64-linux-gnu/sys/types.h \
  /usr/include/x86_64-linux-gnu/bits/types/clock_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/time_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/timer_t.h \
  /usr/include/x86_64-linux-gnu/sys/select.h \
  /usr/include/x86_64-linux-gnu/bits/select.h \
  /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h \
  /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h \
  /usr/include/x86_64-linux-gnu/bits/select2.h \
  /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h \
  /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h \
  /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h \
  /usr/include/x86_64-linux-gnu/bits/atomic_wide_counter.h \
  /usr/include/x86_64-linux-gnu/bits/struct_mutex.h \
  /usr/include/x86_64-linux-gnu/bits/struct_rwlock.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi/asm-generic/ioctls.h \
  /usr/include/linux/ioctl.h \
  /usr/include/x86_64-linux-gnu/asm/ioctl.h \
  /usr/include/asm-generic/ioctl.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/drm_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/sndrv_pcm_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/sndrv_ctl_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/kvm_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/vhost_virtio_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/perf_ioctl_array.c \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/trace/beauty/generated/ioctl/usbdevfs_ioctl_array.c \

trace/beauty/ioctl.o: $(deps_trace/beauty/ioctl.o)

$(deps_trace/beauty/ioctl.o):
