#include "600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        unsigned key = (d->values[i & 63u] + i) % 101u;
        const unsigned* p = d->state;
        while (p != d->state + 4 && key > *p) ++p;
        sum += static_cast<unsigned>(p - d->state);
    }
    return sum;
}
