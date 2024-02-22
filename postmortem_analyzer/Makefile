include ../tmon_perf_6.8/tools/tmon_perf/shared-config.mk

CXX=../llvm-project-17.0.6.src/build/bin/clang++

TMONPERF_DIR=../tmon_perf_6.8/tools/tmon_perf

PTWR_INSTR_H=../llvm-project-17.0.6.src/llvm/lib/Transforms/Instrumentation

TSAN_RTL_H=../llvm-project-17.0.6.src/compiler-rt/lib/tsan/rtl
SAN_COMM_H =../llvm-project-17.0.6.src/compiler-rt/lib/
INC2=../llvm-project-17.0.6.src/compiler-rt/lib/
TSANLIB=../llvm-project-17.0.6.src/build/lib/clang/17/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan.a
SANINT=../llvm-project-17.0.6.src/compiler-rt/lib/sanitizer_common/

tmon_pma.o: tmon_pma.cpp
	$(CXX) -std=c++17 -c -g -o tmon_pma.o tmon_pma.cpp -I $(PTWR_INSTR_H) -I $(TSAN_RTL_H) -I $(SAN_COMM_H) -I $(INC2) -I $(SANINT)

tmon_eh.o: tmon_eh.cpp
	$(CXX) -std=c++17 -c -g -o tmon_eh.o tmon_eh.cpp -I $(PTWR_INSTR_H) -I $(TSAN_RTL_H) -I $(SAN_COMM_H) -I $(INC2) -I $(SANINT)

pma: $(PERFLIBS) $(TMONPERF_DIR)/$(PERF_IN) $(TMONPERF_DIR)/$(PMU_EVENTS_IN) tmon_pma.o tmon_eh.o
	$(CXX) -std=c++17 tmon_pma.o tmon_eh.o $(TSANLIB) $(LDFLAGS) \
	$(TMONPERF_DIR)/$(PERF_IN) $(TMONPERF_DIR)/$(PMU_EVENTS_IN) $(LIBS) -o pma
# $(CFLAGS) after $(CXX)	

.PHONY: clean
clean:
	rm -f pma *.o
