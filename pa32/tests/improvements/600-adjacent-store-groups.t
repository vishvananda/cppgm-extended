#include "../kernels/600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    for (unsigned r = 0; r < n; ++r) {
        d->z[0] = d->x[0] + d->y[0]; d->z[1] = d->x[1] + d->y[1];
        d->z[2] = d->x[0] - d->y[0]; d->z[3] = d->x[1] - d->y[1];
        d->x[r & 1u] += 0.25;
    }
    return 0;
}
