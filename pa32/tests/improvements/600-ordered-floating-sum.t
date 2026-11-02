#include "../kernels/600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    for (unsigned r = 0; r < n; ++r) {
        double s = d->z[0];
        for (unsigned i = 0; i < d->length; ++i) s += d->x[i] * d->y[i];
        d->z[0] = s;
    }
    return 0;
}
