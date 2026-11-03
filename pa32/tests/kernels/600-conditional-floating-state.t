#include "600-support.h"

// Masked lanes retain both floating values on untaken paths. The scalar
// oracle uses a lane loop; the fixed-width kernel exposes their live ranges.
extern "C" unsigned quality_kernel(QualityData* d, unsigned n) {
    unsigned sum = 0;
    for (unsigned r = 0; r < n; ++r) {
        double a[8], b[8];
        for (unsigned k = 0; k < 8; ++k) { a[k] = d->x[k]; b[k] = d->y[k]; }
        unsigned active = 255u ^ d->alias;
        for (unsigned step = 0; step < 16 && active; ++step) {
            if (active & 1u) {
                const double aa = a[0] * a[0], bb = b[0] * b[0];
                const double ab = a[0] * b[0];
                a[0] = aa - bb + d->x[0];
                b[0] = 2.0 * ab + d->y[0];
                if (aa + bb > 4.0) active &= ~1u;
            }
            if (active & 2u) {
                const double aa = a[1] * a[1], bb = b[1] * b[1];
                const double ab = a[1] * b[1];
                a[1] = aa - bb + d->x[1];
                b[1] = 2.0 * ab + d->y[1];
                if (aa + bb > 4.0) active &= ~2u;
            }
            if (active & 4u) {
                const double aa = a[2] * a[2], bb = b[2] * b[2];
                const double ab = a[2] * b[2];
                a[2] = aa - bb + d->x[2];
                b[2] = 2.0 * ab + d->y[2];
                if (aa + bb > 4.0) active &= ~4u;
            }
            if (active & 8u) {
                const double aa = a[3] * a[3], bb = b[3] * b[3];
                const double ab = a[3] * b[3];
                a[3] = aa - bb + d->x[3];
                b[3] = 2.0 * ab + d->y[3];
                if (aa + bb > 4.0) active &= ~8u;
            }
            if (active & 16u) {
                const double aa = a[4] * a[4], bb = b[4] * b[4];
                const double ab = a[4] * b[4];
                a[4] = aa - bb + d->x[4];
                b[4] = 2.0 * ab + d->y[4];
                if (aa + bb > 4.0) active &= ~16u;
            }
            if (active & 32u) {
                const double aa = a[5] * a[5], bb = b[5] * b[5];
                const double ab = a[5] * b[5];
                a[5] = aa - bb + d->x[5];
                b[5] = 2.0 * ab + d->y[5];
                if (aa + bb > 4.0) active &= ~32u;
            }
            if (active & 64u) {
                const double aa = a[6] * a[6], bb = b[6] * b[6];
                const double ab = a[6] * b[6];
                a[6] = aa - bb + d->x[6];
                b[6] = 2.0 * ab + d->y[6];
                if (aa + bb > 4.0) active &= ~64u;
            }
            if (active & 128u) {
                const double aa = a[7] * a[7], bb = b[7] * b[7];
                const double ab = a[7] * b[7];
                a[7] = aa - bb + d->x[7];
                b[7] = 2.0 * ab + d->y[7];
                if (aa + bb > 4.0) active &= ~128u;
            }
        }
        sum += active ^ r;
        for (unsigned k = 0; k < 8; ++k) d->z[k] = a[k] + b[k];
    }
    return sum;
}
