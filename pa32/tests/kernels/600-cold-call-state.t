#include "600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned a = 1, b = 3, c = 5, e = 7, sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        a += i; b ^= a + (b >> 2); c += b * 17u; e ^= c + 3u;
        if ((i & 63u) == 17u) c += quality_cold(d, i);
        sum += (a ^ b) + (c ^ e);
    }
    return sum;
}
