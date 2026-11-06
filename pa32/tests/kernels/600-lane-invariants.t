#include "600-support.h"
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    const double scale = 0.99999997 + static_cast<double>(d->alias) * 0.00000001;
    const volatile double* observed = d->alias == 0 ? d->z : d->alias == 1 ? d->z + 1 : d->y;
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        d->z[0] *= scale;
        d->z[1] *= scale;
        sum += *observed > 0.0;
    }
    return sum;
}
