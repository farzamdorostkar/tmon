cmd_util/perf-in.o :=  ld   -r -o util/perf-in.o  util/arm64-frame-pointer-unwind-support.o util/addr_location.o util/annotate.o util/block-info.o util/block-range.o util/build-id.o util/cacheline.o util/config.o util/copyfile.o util/ctype.o util/db-export.o util/env.o util/event.o util/evlist.o util/sideband_evlist.o util/evsel.o util/evsel_fprintf.o util/perf_event_attr_fprintf.o util/evswitch.o util/find_bit.o util/get_current_dir_name.o util/levenshtein.o util/mmap.o util/memswap.o util/parse-events.o util/print-events.o util/tracepoint.o util/perf_regs.o util/perf-regs-arch/perf-in.o util/path.o util/print_binary.o util/rlimit.o util/argv_split.o util/rbtree.o util/libstring.o util/bitmap.o util/hweight.o util/smt.o util/strbuf.o util/string.o util/strlist.o util/strfilter.o util/top.o util/usage.o util/dso.o util/dsos.o util/symbol.o util/symbol_fprintf.o util/map_symbol.o util/color.o util/color_config.o util/metricgroup.o util/header.o util/callchain.o util/values.o util/debug.o util/fncache.o util/machine.o util/map.o util/maps.o util/pstack.o util/session.o util/sample-raw.o util/s390-sample-raw.o util/amd-sample-raw.o util/syscalltbl.o util/ordered-events.o util/namespaces.o util/comm.o util/thread.o util/thread_map.o util/parse-events-flex.o util/parse-events-bison.o util/pmu.o util/pmus.o util/pmu-flex.o util/pmu-bison.o util/svghelper.o util/trace-event-info.o util/trace-event-scripting.o util/trace-event.o util/trace-event-parse.o util/trace-event-read.o util/sort.o util/hist.o util/util.o util/cpumap.o util/affinity.o util/cputopo.o util/cgroup.o util/target.o util/rblist.o util/intlist.o util/vdso.o util/counts.o util/stat.o util/stat-shadow.o util/stat-display.o util/perf_api_probe.o util/record.o util/srcline.o util/srccode.o util/synthetic-events.o util/data.o util/tsc.o util/cloexec.o util/call-path.o util/rwsem.o util/thread-stack.o util/spark.o util/topdown.o util/iostat.o util/stream.o util/auxtrace.o util/intel-pt-decoder/perf-in.o util/intel-pt.o util/intel-bts.o util/arm-spe.o util/arm-spe-decoder/perf-in.o util/hisi-ptt.o util/hisi-ptt-decoder/perf-in.o util/s390-cpumsf.o util/cs-etm-base.o util/parse-branch-options.o util/dump-insn.o util/parse-regs-options.o util/parse-sublevel-options.o util/term.o util/help-unknown-cmd.o util/dlfilter.o util/mem-events.o util/vsprintf.o util/units.o util/time-utils.o util/expr-flex.o util/expr-bison.o util/expr.o util/branch.o util/mem2node.o util/clockid.o util/list_sort.o util/mutex.o util/sharded_mutex.o util/bpf_map.o util/bpf_counter.o util/bpf_counter_cgroup.o util/bpf_ftrace.o util/bpf_off_cpu.o util/bpf-filter.o util/bpf-filter-flex.o util/bpf-filter-bison.o util/bpf_lock_contention.o util/bpf_kwork.o util/bpf_kwork_top.o util/symbol-elf.o util/probe-file.o util/probe-event.o util/probe-finder.o util/dwarf-aux.o util/dwarf-regs.o util/debuginfo.o util/annotate-data.o util/unwind-libunwind-local.o util/unwind-libunwind.o util/data-convert-bt.o util/data-convert-json.o util/scripting-engines/perf-in.o util/zlib.o util/lzma.o util/zstd.o util/cap.o util/demangle-cxx.o util/demangle-ocaml.o util/demangle-java.o util/demangle-rust.o util/jitdump.o util/genelf.o util/genelf_debug.o util/perf-hooks.o util/bpf-event.o util/bpf-utils.o util/pfm.o