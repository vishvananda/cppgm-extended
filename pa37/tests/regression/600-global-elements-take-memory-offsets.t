// An element of a global array is read and written with the array's address
// in the memory immediate: the scaled index alone is the base. A field of an
// element adds its offset to that immediate.
struct Pair { unsigned key; unsigned value; };
static unsigned table[64];
static unsigned char bytes[256];
static Pair pairs[32];
extern "C" __attribute__((noinline)) void fill(unsigned seed) {
    for (unsigned i = 0; i < 64; ++i) {
        seed = seed * 1664525u + 1013904223u;
        table[i] = seed;
        bytes[i * 3] = (unsigned char)(seed >> 9);
    }
}
extern "C" __attribute__((noinline)) unsigned pick(unsigned i) {
    return table[i & 63] + bytes[i & 255];
}
extern "C" __attribute__((noinline)) void put(unsigned i, unsigned value) {
    pairs[i & 31].value = value;
    pairs[i & 31].key = i;
}
extern "C" __attribute__((noinline)) unsigned value_of(unsigned i) {
    return pairs[i & 31].value + pairs[i & 31].key;
}
int main() {
    fill(42);
    if (pick(0) != 1083814273u + 216) return 1;
    if (pick(63 + 64) != table[63] + bytes[127]) return 2;
    put(37, 1000);
    if (value_of(5) != 1037) return 3;
    put(31, 7);
    return value_of(31) != 38;
}
