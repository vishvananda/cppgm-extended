// Copying across conditional boundaries preserves zero-trip guards, overlap,
// narrow/wide counts and observable stores.
template<class Count>
__attribute__((noinline)) void copy_segments(int** node, int* dst, const int* src, Count n) {
    while (n--) {
        *dst = *src++;
        ++dst;
        if (dst != *node + 8) continue;
        ++node; dst = *node;
    }
}
__attribute__((noinline)) void copy_observed(int** node, volatile int* dst, const int* src, unsigned n) {
    while (n--) {
        *dst = *src++;
        ++dst;
        if (dst == *node + 8) { ++node; dst = *node; }
    }
}
// Byte writes may alias the pointer table. Changing the selected pointer on
// the first iteration changes the boundary on all following iterations.
__attribute__((noinline)) int byte_alias(int** node, int* dst, int* replacement, unsigned n) {
    int switches = 0;
    while (n--) {
        unsigned char* out = reinterpret_cast<unsigned char*>(node);
        const unsigned char* in = reinterpret_cast<const unsigned char*>(&replacement);
        for (unsigned i = 0; i < sizeof(replacement); ++i) out[i] = in[i];
        ++dst;
        if (dst == *node + 8) { ++node; dst = *node; ++switches; }
    }
    return switches;
}
int main() {
    copy_segments<unsigned>(nullptr, nullptr, nullptr, 0);
    copy_segments<unsigned long>(nullptr, nullptr, nullptr, 0);
    copy_segments<int>(nullptr, nullptr, nullptr, 0);
    copy_observed(nullptr, nullptr, nullptr, 0);
    for (unsigned count = 0; count <= 24; ++count) {
        int a[8] = {}, b[8] = {}, c[8] = {}, spare[8] = {}, source[24];
        int* nodes[] = {a, b, c, spare};
        for (unsigned i = 0; i < 24; ++i) source[i] = int(i * 17 + 3);
        copy_segments(nodes, a, source, count);
        for (unsigned i = 0; i < 24; ++i)
            if (nodes[i / 8][i % 8] != (i < count ? source[i] : 0)) return 1;
        copy_segments(nodes, a, source, static_cast<unsigned long>(count));
        copy_segments(nodes, a, source, static_cast<int>(count));
        copy_observed(nodes, a, source, count);
        for (unsigned i = 0; i < count; ++i)
            if (nodes[i / 8][i % 8] != source[i]) return 2;
    }
    int shared[32];
    for (unsigned i = 0; i < 32; ++i) shared[i] = int(i + 1);
    int* overlap[] = {shared, shared + 8, shared + 16, shared + 24};
    copy_segments(overlap, shared + 1, shared, 20u);
    for (unsigned i = 0; i <= 20; ++i) if (shared[i] != 1) return 3;
    int* changing[] = {shared, shared + 16, shared + 24};
    if (byte_alias(changing, shared, shared + 4, 9) != 0) return 4;
    return 0;
}
