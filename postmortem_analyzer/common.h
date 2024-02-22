#ifndef DECODER_API_H
#define DECODER_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t u64t;
typedef uint32_t u32t;

struct synth_intel_ptwrite {
  uint32_t pid, tid;
  uint64_t payload;
};

int tmon_perf(int argc, const char **argv);
void tmon_pass_event(struct synth_intel_ptwrite *sample);

#ifdef __cplusplus
}
#endif

#endif /* DECODER_API_H */