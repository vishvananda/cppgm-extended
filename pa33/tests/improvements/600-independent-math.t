#include "../kernels/600-support.h"
extern "C" double sqrt(double);
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    for (unsigned r = 0; r < n; ++r) {
        double a = sqrt(d->x[0]*d->x[0]+1), b = sqrt(d->x[1]*d->x[1]+1);
        double c = sqrt(d->x[2]*d->x[2]+1), e = sqrt(d->x[3]*d->x[3]+1);
        d->z[0] = 1/a + 1/b + 1/c + 1/e;
        d->x[r & 3u] += 0.0009765625;
    }
    return 0;
}
