CC=../../llvm-project-17.0.6.src/build/bin/clang
OPT=../../llvm-project-17.0.6.src/build/bin/opt

CFLAGS=-g -O1

TMON_INTERC=../interceptor_lib

# Compile the source code to LLVM IR
tiny_race.ll: tiny_race.c
	$(CC) $(CFLAGS) -S -emit-llvm $^ -o $@

# Apply the 'tmon' LLVM pass to the generated LLVM IR
tiny_race_tmon.ll: tiny_race.ll
	$(OPT) -passes='tmon' -S $^ -o $@

# Compile the transformed LLVM IR to executable
tiny_race_tmon: tiny_race_tmon.ll
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -L $(TMON_INTERC) -l:libtmon_interceptors.a

tiny_race_tsan.ll: tiny_race.c
	$(CC) $(CFLAGS) -fsanitize=thread -S -emit-llvm $^ -o $@

.PHONY: clean
clean:
	rm -f tiny_race_tmon *.ll