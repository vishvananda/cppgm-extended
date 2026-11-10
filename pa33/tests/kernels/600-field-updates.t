#include "600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned* a = d->values;
    unsigned* b = d->alias ? a : d->other;
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        unsigned before = *a; *b = before + (i & 7u); sum = (sum * 33u) ^ (*a + before + i);
    }
    return sum;
}
