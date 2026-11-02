#include "600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned a = d->state[0], sum = 0; unsigned char b = 19;
    for (unsigned i = 0; i < n; ++i) {
        a = (a * 33u + i) ^ (a >> 5);
        b = static_cast<unsigned char>((b << 1) ^ (a >> 3));
        sum += d->values[a & 63u] + b + a / 7u + a % 7u;
    }
    d->state[0] = a; return sum;
}
