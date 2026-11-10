#ifndef CPPGM_QUALITY_SUPPORT_H
#define CPPGM_QUALITY_SUPPORT_H
struct QualityData {
    unsigned values[72], other[72], state[4], calls, alias, length;
    unsigned char bytes[80];
    double x[72], y[72], z[72];
};
extern "C" unsigned quality_kernel(QualityData*, unsigned);
extern "C" unsigned quality_cold(QualityData*, unsigned) noexcept;
#ifdef QUALITY_DRIVER
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
extern "C" unsigned quality_dirty(QualityData*, unsigned);
// System V leaves the upper half of an unsigned argument unspecified.
// This host-only tail transfer supplies nonzero bits there without changing
// the value, call alignment or any callee-saved register.
asm(".text\n.globl quality_dirty\n.type quality_dirty,@function\n"
    "quality_dirty:\nmovabs $0x5a5a123400000000,%rax\nor %rax,%rsi\n"
    "jmp quality_kernel\n.size quality_dirty,.-quality_dirty\n");
extern "C" unsigned quality_cold(QualityData* d, unsigned i) noexcept {
    ++d->calls; d->state[2] = d->state[2] * 33u + i;
    return d->state[2] ^ d->calls;
}
static unsigned oracle(QualityData* d, unsigned n) {
    unsigned sum = 0;
#if QUALITY_CASE == 1
    for (unsigned i = 0; i < n; ++i) {
        unsigned key = (d->values[i & 63u] + i) % 101u, j = 0;
        while (j < 4 && key > d->state[j]) ++j;
        sum += j;
    }
#elif QUALITY_CASE == 2
    unsigned* a = d->values;
    unsigned* b = d->alias ? a : d->other;
    for (unsigned i = 0; i < n; ++i) {
        unsigned before = *a; *b = before + (i & 7u);
        sum = (sum * 33u) ^ (*a + before + i);
    }
#elif QUALITY_CASE == 3
    unsigned a = d->state[0]; unsigned char b = 19;
    for (unsigned i = 0; i < n; ++i) {
        a = (a * 33u + i) ^ (a >> 5);
        b = static_cast<unsigned char>((b << 1) ^ (a >> 3));
        sum += d->values[a & 63u] + b + a / 7u + a % 7u;
    }
    d->state[0] = a;
#elif QUALITY_CASE == 4
    unsigned a = 1, b = 3, c = 5, e = 7;
    for (unsigned i = 0; i < n; ++i) {
        a += i; b ^= a + (b >> 2); c += b * 17u; e ^= c + 3u;
        if ((i & 63u) == 17u) c += quality_cold(d, i);
        sum += (a ^ b) + (c ^ e);
    }
#elif QUALITY_CASE == 5
    double* out = d->alias == 1 ? d->x + 1 : (d->alias == 2 || d->alias == 3) ? d->x : d->z;
    const double* in = d->alias == 3 ? d->x + 1 : d->x;
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned i = 0; i < d->length; ++i) out[i] = in[i] * d->y[i] + 0.5;
        d->x[r & 63u] += 0.0009765625;
    }
#elif QUALITY_CASE == 6
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned i = 0; i < d->length; ++i) {
            d->bytes[i] = static_cast<unsigned char>((d->values[i] * 3u) + d->other[i]);
            sum += d->bytes[i];
        }
        d->values[r & 63u] += r;
    }
#elif QUALITY_CASE == 7
    for (unsigned r = 0; r < n; ++r) {
        d->z[0] = d->x[0] + d->y[0]; d->z[1] = d->x[1] + d->y[1];
        d->z[2] = d->x[0] - d->y[0]; d->z[3] = d->x[1] - d->y[1];
        d->x[r & 1u] += 0.25;
    }
#elif QUALITY_CASE == 8
    for (unsigned r = 0; r < n; ++r) {
        for (unsigned j = 0; j < 4; ++j) {
            double s = 0;
            for (unsigned k = 0; k < 4; ++k) s += d->x[k] * d->y[k*4+j];
            d->z[j] = s;
        }
        d->x[r & 3u] += 0.25;
    }
#elif QUALITY_CASE == 9
    for (unsigned r = 0; r < n; ++r) {
        double a = std::sqrt(d->x[0]*d->x[0]+1), b = std::sqrt(d->x[1]*d->x[1]+1);
        double c = std::sqrt(d->x[2]*d->x[2]+1), e = std::sqrt(d->x[3]*d->x[3]+1);
        d->z[0] = 1/a + 1/b + 1/c + 1/e;
        d->x[r & 3u] += 0.0009765625;
    }
#elif QUALITY_CASE == 10
    for (unsigned r = 0; r < n; ++r) {
        double s = d->z[0];
        for (unsigned i = 0; i < d->length; ++i) s += d->x[i] * d->y[i];
        d->z[0] = s;
    }
#elif QUALITY_CASE == 11
    for (unsigned r = 0; r < n; ++r) {
        double a[8], b[8];
        for (unsigned k = 0; k < 8; ++k) { a[k] = d->x[k]; b[k] = d->y[k]; }
        unsigned active = 255u ^ d->alias;
        for (unsigned step = 0; step < 16 && active; ++step) {
            for (unsigned k = 0; k < 8; ++k) {
                if (active & (1u << k)) {
                    const double aa = a[k] * a[k], bb = b[k] * b[k];
                    const double ab = a[k] * b[k];
                    a[k] = aa - bb + d->x[k];
                    b[k] = 2.0 * ab + d->y[k];
                    if (aa + bb > 4.0) active &= ~(1u << k);
                }
            }
        }
        sum += active ^ r;
        for (unsigned k = 0; k < 8; ++k) d->z[k] = a[k] + b[k];
    }
#elif QUALITY_CASE == 12
    const double scale = 0.99999997 + static_cast<double>(d->alias) * 0.00000001;
    const volatile double* observed = d->alias == 0 ? d->z : d->alias == 1 ? d->z + 1 : d->y;
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned lane = 0; lane < 2; ++lane) d->z[lane] *= scale;
        sum += *observed > 0.0;
    }
#endif
    return sum;
}
static unsigned mix(unsigned hash, unsigned value) { return (hash ^ value) * 16777619u; }
static unsigned digest(const QualityData& d) {
    unsigned h = 2166136261u;
    for (unsigned i = 0; i < 72; ++i) {
        h = mix(mix(h, d.values[i]), d.other[i]);
        const double v[] = {d.x[i], d.y[i], d.z[i]};
        for (unsigned k = 0; k < 3; ++k) {
            unsigned long long bits; std::memcpy(&bits, &v[k], sizeof bits);
            // Arithmetic need not preserve a NaN's payload. Copy-only bit
            // preservation has separate representation fixtures.
            if ((bits & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL &&
                (bits & 0xfffffffffffffULL)) bits = 0x7ff8000000000000ULL;
            h = mix(mix(h, static_cast<unsigned>(bits)), static_cast<unsigned>(bits >> 32));
        }
    }
    for (unsigned i = 0; i < 4; ++i) h = mix(h, d.state[i]);
    for (unsigned i = 0; i < 80; ++i) h = mix(h, d.bytes[i]);
    return mix(h, d.calls);
}
int main(int argc, char** argv) {
    if (argc != 4) return 2;
    unsigned n = std::strtoul(argv[2], 0, 10), scenario = std::strtoul(argv[3], 0, 10);
    QualityData d = {};
    d.alias = scenario & 3u; d.length = scenario >> 2;
    if (d.length > 68) return 3;
    for (unsigned i = 0; i < 72; ++i) {
        d.values[i] = i * 19u + 0xfffffff0u; d.other[i] = ~d.values[i];
        d.x[i] = (static_cast<int>(i % 9) - 4) * 0.25;
        d.y[i] = (i % 7) * 0.125; d.z[i] = -0.0;
    }
    d.state[0] = 17; d.state[1] = 43; d.state[2] = 71; d.state[3] = 99;
#if QUALITY_CASE == 12
    d.z[0] = 0.75; d.z[1] = -1.25;
#endif
#if QUALITY_CASE == 10
    // A reassociated reduction would lose the intervening small term.
    d.x[0] = 1e16; d.x[1] = 1; d.x[2] = -1e16;
    d.y[0] = d.y[1] = d.y[2] = 1;
#endif
    unsigned value = argv[1][0] == 'o' ? oracle(&d, n) : quality_dirty(&d, n);
    std::printf("%u:%u:%u\n", value, digest(d), d.calls);
}
#endif
#endif
