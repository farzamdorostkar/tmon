cmd_tests/unit_number__scnprintf.o := gcc -Wp,-MD,tests/.unit_number__scnprintf.o.d -Wp,-MT,tests/unit_number__scnprintf.o -Wbad-function-cast -Wdeclaration-after-statement -Wformat-security -Wformat-y2k -Winit-self -Wmissing-declarations -Wmissing-prototypes -Wno-system-headers -Wold-style-definition -Wpacked -Wredundant-decls -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wundef -Wwrite-strings -Wformat -Wno-type-limits -Wstrict-aliasing=3 -Wshadow -DHAVE_SYSCALL_TABLE_SUPPORT -DHAVE_ARCH_X86_64_SUPPORT -Iarch/x86/include/generated -DHAVE_PERF_REGS_SUPPORT -DHAVE_ARCH_REGS_QUERY_REGISTER_OFFSET -Werror -DNDEBUG=1 -O6 -fno-omit-frame-pointer -Wall -Wextra -std=gnu11 -fstack-protector-all -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/arch/x86/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/ -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf -DHAVE_PTHREAD_ATTR_SETAFFINITY_NP -DHAVE_PTHREAD_BARRIER -DHAVE_EVENTFD_SUPPORT -DHAVE_GET_CURRENT_DIR_NAME -DHAVE_GETTID -DHAVE_FILE_HANDLE -DHAVE_DWARF_GETLOCATIONS_SUPPORT -DHAVE_DWARF_CFI_SUPPORT -DHAVE_AIO_SUPPORT -DHAVE_SCANDIRAT_SUPPORT -DHAVE_SCHED_GETCPU_SUPPORT -DHAVE_SETNS_SUPPORT -DHAVE_ZLIB_SUPPORT -DHAVE_LIBELF_SUPPORT -DHAVE_ELF_GETPHDRNUM_SUPPORT -DHAVE_GELF_GETNOTE_SUPPORT -DHAVE_ELF_GETSHDRSTRNDX_SUPPORT -DHAVE_DWARF_SUPPORT -DHAVE_LIBBPF_SUPPORT -DHAVE_SDT_EVENT -DHAVE_JITDUMP -DHAVE_BPF_SKEL -DHAVE_DWARF_UNWIND_SUPPORT -DNO_LIBUNWIND_DEBUG_FRAME -DHAVE_LIBUNWIND_SUPPORT -DHAVE_LIBCRYPTO_SUPPORT -DHAVE_SLANG_SUPPORT -DHAVE_LIBPERL_SUPPORT -DHAVE_TIMERFD_SUPPORT -DHAVE_LIBPYTHON_SUPPORT -DHAVE_CXA_DEMANGLE_SUPPORT -DHAVE_LZMA_SUPPORT -DHAVE_ZSTD_SUPPORT -DHAVE_LIBCAP_SUPPORT -DHAVE_BACKTRACE_SUPPORT -DHAVE_LIBNUMA_SUPPORT -DHAVE_KVM_STAT_SUPPORT -DDISASM_FOUR_ARGS_SIGNATURE -DHAVE_PERF_READ_VDSO32 -DHAVE_PERF_READ_VDSOX32 -DHAVE_LIBBABELTRACE_SUPPORT -DHAVE_AUXTRACE_SUPPORT -DHAVE_JVMTI_CMLR -DHAVE_LIBPFM -DHAVE_LIBTRACEEVENT -DLIBTRACEEVENT_VERSION=65793 -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libapi/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libbpf/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libsubcmd/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libsymbol/include -I/home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/libperf/include -D"BUILD_STR(s)=$(pound)s" -c -o tests/unit_number__scnprintf.o tests/unit_number__scnprintf.c

source_tests/unit_number__scnprintf.o := tests/unit_number__scnprintf.c

deps_tests/unit_number__scnprintf.o := \
  /usr/include/stdc-predef.h \
  /usr/include/inttypes.h \
  /usr/include/features.h \
  /usr/include/features-time64.h \
  /usr/include/x86_64-linux-gnu/bits/wordsize.h \
  /usr/include/x86_64-linux-gnu/bits/timesize.h \
  /usr/include/x86_64-linux-gnu/sys/cdefs.h \
  /usr/include/x86_64-linux-gnu/bits/long-double.h \
  /usr/include/x86_64-linux-gnu/gnu/stubs.h \
  /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdint.h \
  /usr/include/stdint.h \
  /usr/include/x86_64-linux-gnu/bits/libc-header-start.h \
  /usr/include/x86_64-linux-gnu/bits/types.h \
  /usr/include/x86_64-linux-gnu/bits/typesizes.h \
  /usr/include/x86_64-linux-gnu/bits/time64.h \
  /usr/include/x86_64-linux-gnu/bits/wchar.h \
  /usr/include/x86_64-linux-gnu/bits/stdint-intn.h \
  /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler_types.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/compiler-gcc.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/linux/types.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdbool.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stddef.h \
  /usr/include/x86_64-linux-gnu/asm/types.h \
  /usr/include/asm-generic/types.h \
  /usr/include/asm-generic/int-ll64.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/arch/x86/include/uapi/asm/bitsperlong.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/asm-generic/bitsperlong.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/include/uapi/asm-generic/bitsperlong.h \
  /usr/include/x86_64-linux-gnu/asm/posix_types.h \
  /usr/include/x86_64-linux-gnu/asm/posix_types_64.h \
  /usr/include/asm-generic/posix_types.h \
  /usr/include/string.h \
  /usr/include/x86_64-linux-gnu/bits/types/locale_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h \
  /usr/include/strings.h \
  /usr/include/x86_64-linux-gnu/bits/strings_fortified.h \
  /usr/include/x86_64-linux-gnu/bits/string_fortified.h \
  tests/tests.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/units.h \
  /home/farzam/code/tmon_test/tmon_perf_6.8/tools/perf/util/debug.h \
  /usr/lib/gcc/x86_64-linux-gnu/11/include/stdarg.h \
  /usr/include/stdio.h \
  /usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h \
  /usr/include/x86_64-linux-gnu/bits/types/__FILE.h \
  /usr/include/x86_64-linux-gnu/bits/types/FILE.h \
  /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h \
  /usr/include/x86_64-linux-gnu/bits/types/cookie_io_functions_t.h \
  /usr/include/x86_64-linux-gnu/bits/stdio_lim.h \
  /usr/include/x86_64-linux-gnu/bits/floatn.h \
  /usr/include/x86_64-linux-gnu/bits/floatn-common.h \
  /usr/include/x86_64-linux-gnu/bits/stdio.h \
  /usr/include/x86_64-linux-gnu/bits/stdio2.h \

tests/unit_number__scnprintf.o: $(deps_tests/unit_number__scnprintf.o)

$(deps_tests/unit_number__scnprintf.o):