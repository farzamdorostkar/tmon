This repository is home to ThreadMonitor (TMon), a low-overhead postmortem data race detector for C/C++ programs that use the Pthread Library. TMon uses compile-time instrumentation to trace the information required for detecting data races at runtime (i.e. memory accesses and synchronization events) using Intel Processor Trace (PT), a non-intrusive hardware feature dedicated to tracing software execution. Thereafter, a post-mortem analyzer uses the trace data to determine whether
the traced program execution exhibited data races.

1 - Build the specialized LLVM 17 that comes with the project. It is equipped with TMon's compile-time instrumentation framework, already registered with the generated `opt` tool.

    cd llvm-project-17.0.6.src
    mkdir build
    cd build
    cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" -DCMAKE_BUILD_TYPE="Release" -DLLVM_BUILD_EXAMPLES=1 -DCLANG_BUILD_EXAMPLES=1 -G "Unix Makefiles" ../llvm

2 - Build TMon's interceptor library. It is a static library containing interceptors for Pthread functions.

    cd interceptor_lib
    make libtmon_interceptors.a

3 - Build the specialized perf 6.8 that comes with the project. It is equipped with the required API to be used alongside our postmortem analyzer as the decoder. from the root directory run

    PYTHON=python3 make -C tmon_perf_6.8/tools/perf

4 - Build the postmortem analyzer. From the root directory run

    cd postmortem_analyzer
    make pma

To test the tool, you can use the test program located in the `test` directory.

    cd test
    make tiny_race_tmon

Run it under `perf`, with tracing of `ptwrite` packets enabled.

    perf record --no-bpf-event -e intel_pt/ptw,branch=0,mtc=0,cyc=0,pwr_evt=0,percore=0,noretcomp=0/u ./tiny_race_tmon

A `perf.data` file will be generated containing the trace data. Pass this file to the postmortem analyzer using the following command:

    ./pma script --itrace=ew -F tid,pid,synth --input=path/to/tracedata
