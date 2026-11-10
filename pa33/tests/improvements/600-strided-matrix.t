#include "../kernels/600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned j = 0; j < 4; ++j) {
            double s = 0;
            for (unsigned k = 0; k < 4; ++k) s += d->x[k] * d->y[k*4+j];
            d->z[j] = s;
        }
        d->x[r & 3u] += 0.25;
    }
    return 0;
}
