#include "../kernels/600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned sum = 0;
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned i = 0; i < d->length; ++i) {
            d->bytes[i] = static_cast<unsigned char>((d->values[i] * 3u) + d->other[i]);
            sum += d->bytes[i];
        }
        d->values[r & 63u] += r;
    }
    return sum;
}
