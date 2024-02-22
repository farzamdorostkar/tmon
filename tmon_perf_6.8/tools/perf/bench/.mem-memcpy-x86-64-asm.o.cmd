cmd_bench/mem-memcpy-x86-64-asm.o := gcc -Wp,-MD,bench/.mem-memcpy-x86-64-asm.o.d -Wp,-MT,bench/mem-memcpy-x86-64-asm.o -Wbad-function-cast -Wdeclaration-after-statement -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-prototypes -Wno-system-headers -Wold-style-definition -Wpacked -Wredundant-decls -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wundef -Wwrite-strings -Wformat -Wno-type-limits -Wstrict-aliasing=3 -Wshadow -DHAVE_SYSCALL_TABLE_SUPPORT -DHAVE_ARCH_X86_64_SUPPORT -Iarch/x86/include/generated -DHAVE_PERF_REGS_SUPPORT -DHAVE_ARCH_REGS_QUERY_REGISTER_OFFSET -Werror -DNDEBUG=1 -O6 -fno-omit-frame-pointer -Wall -Wextra -std=gnu11 -fstack-protector-all -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/arch/x86/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf -DHAVE_PTHREAD_ATTR_SETAFFINITY_NP -DHAVE_PTHREAD_BARRIER -DHAVE_EVENTFD_SUPPORT -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_GETTID -DHAVE_FILE_HANDLE -DHAVE_DWARF_GETLOCATIONS_SUPPORT -DHAVE_DWARF_CFI_SUPPORT -DHAVE_AIO_SUPPORT -DHAVE_SCANDIRAT_SUPPORT -DHAVE_SCHED_GETCPU_SUPPORT -DHAVE_SETNS_SUPPORT -DHAVE_ZLIB_SUPPORT -DHAVE_LIBELF_SUPPORT -DHAVE_ELF_GETPHDRNUM_SUPPORT -DHAVE_GELF_GETNOTE_SUPPORT -DHAVE_ELF_GETSHDRSTRNDX_SUPPORT -DHAVE_DWARF_SUPPORT -DHAVE_LIBBPF_SUPPORT -DHAVE_SDT_EVENT -DHAVE_JITDUMP -DHAVE_BPF_SKEL -DHAVE_DWARF_UNWIND_SUPPORT -DNO_LIBUNWIND_DEBUG_FRAME -DHAVE_LIBUNWIND_SUPPORT -DHAVE_LIBCRYPTO_SUPPORT -DHAVE_SLANG_SUPPORT -DHAVE_LIBPERL_SUPPORT -DHAVE_TIMERFD_SUPPORT -DHAVE_LIBPYTHON_SUPPORT -DHAVE_CXA_DEMANGLE_SUPPORT -DHAVE_LZMA_SUPPORT -DHAVE_ZSTD_SUPPORT -DHAVE_LIBCAP_SUPPORT -DHAVE_BACKTRACE_SUPPORT -DHAVE_LIBNUMA_SUPPORT -DHAVE_KVM_STAT_SUPPORT -DDISASM_FOUR_ARGS_SIGNATURE -DHAVE_PERF_READ_VDSO32 -DHAVE_PERF_READ_VDSOX32 -DHAVE_LIBBABELTRACE_SUPPORT -DHAVE_AUXTRACE_SUPPORT -DHAVE_JVMTI_CMLR -DHAVE_LIBPFM -DHAVE_LIBTRACEEVENT -DLIBTRACEEVENT_VERSION=65793 -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libapi/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libbpf/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libsubcmd/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libsymbol/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libperf/include -D"BUILD_STR(s)=$(pound)s" -c -o bench/mem-memcpy-x86-64-asm.o bench/mem-memcpy-x86-64-asm.S

source_bench/mem-memcpy-x86-64-asm.o := bench/mem-memcpy-x86-64-asm.S

deps_bench/mem-memcpy-x86-64-asm.o := \
  /usr/include/stdc-predef.h \
  bench/../../arch/x86/lib/memcpy_64.S \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/include/linux/linkage.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi/asm/errno.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi/asm-generic/errno.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi/asm-generic/errno-base.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/asm/cpufeatures.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/asm/required-features.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/asm/disabled-features.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/asm/alternative.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/asm/export.h \

bench/mem-memcpy-x86-64-asm.o: $(deps_bench/mem-memcpy-x86-64-asm.o)

$(deps_bench/mem-memcpy-x86-64-asm.o):
