#include <errno.h>

extern "C" void* malloc(unsigned long);
extern "C" void free(void*);
unsigned char* published_output;
volatile unsigned output_events;

static __attribute__((noinline)) unsigned long write_output(unsigned char* out, unsigned long n) {
    for (unsigned long i = 0; i < n; ++i) out[i] = (unsigned char)(i * 7 + 3);
    return n + 5;
}
extern "C" __attribute__((noinline)) unsigned long unused_heap(unsigned long n) {
    unsigned char* p = (unsigned char*)malloc(n);
    unsigned long result = write_output(p, n);
    free(p);
    return result;
}
static __attribute__((noinline)) void publish_output(unsigned char* p) {
    published_output = p;
    p[0] = 19;
}
static __attribute__((noinline)) unsigned two_outputs(unsigned char* a, unsigned char* b) {
    a[0] = 3;
    b[0] = 4;
    return 9;
}
static __attribute__((noinline)) void observable_write(volatile unsigned char* p) {
    p[0] = 7;
    ++output_events;
}
static __attribute__((noinline)) int allocation_errno(unsigned long size) {
    errno = 0;
    void* p = malloc(size);
    int changed = errno != 0;
    free(p);
    return changed;
}
int main() {
    if (unused_heap(0) != 5 || unused_heap(1) != 6 || unused_heap(65) != 70) return 1;
    unsigned char* p = (unsigned char*)malloc(8);
    publish_output(p);
    if (published_output[0] != 19) return 2;
    if (two_outputs(p, p) != 9 || p[0] != 4) return 3;
    observable_write(p);
    if (p[0] != 7 || output_events != 1) return 4;
    free(p);
    if (allocation_errno(16) != 0 || allocation_errno(~0UL >> 1) != 1) return 5;
    return 0;
}
