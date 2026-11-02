#include "../kernels/600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    double* out = d->alias == 1 ? d->x + 1 : d->alias == 2 ? d->x : d->z;
    const double* in = d->alias == 3 ? d->x + 1 : d->x;
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned i = 0; i < d->length; ++i) out[i] = in[i] * d->y[i] + 0.5;
        d->x[r & 63u] += 0.0009765625;
    }
    return 0;
}
