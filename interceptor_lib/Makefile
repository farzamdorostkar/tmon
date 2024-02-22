# Build libtmon_interceptors.a

CXX=g++

.PHONY: all clean
all: libtmon_interceptors.a

tmon_interceptors.o: tmon_interceptors.cpp
	$(CXX) -c -fPIC -o tmon_interceptors.o tmon_interceptors.cpp -DTMON_DEBUG_MODE
# compile with -DTMON_DEBUG_MODE for debugging mode

libtmon_interceptors.a: tmon_interceptors.o
	ar -rcs libtmon_interceptors.a tmon_interceptors.o

clean:
	rm -f libtmon_interceptors.a *.o