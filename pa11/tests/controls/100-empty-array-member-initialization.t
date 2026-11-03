// Empty member lists zero integral elements without changing neighboring
// members, constructor effects or nonzero prefixes.
// expect exit 0
int constructed;
struct Element { int value; Element() : value(++constructed) {} };
struct Members {
    int before = 17;
    unsigned char bytes[128] = {};
    int after = 29;
    int integers[32] = {};
    bool flags[32] = {};
    volatile unsigned char observed[32] = {};
    int prefix[32] = {7, 11};
    Element objects[8] = {};
};
int main() {
    Members m;
    if (m.before != 17 || m.after != 29 || constructed != 8) return 1;
    for (unsigned i = 0; i < 128; ++i) if (m.bytes[i]) return 2;
    for (unsigned i = 0; i < 32; ++i) {
        if (m.integers[i] || m.flags[i] || m.observed[i]) return 3;
        if (m.prefix[i] != (i == 0 ? 7 : i == 1 ? 11 : 0)) return 4;
    }
    for (unsigned i = 0; i < 8; ++i)
        if (m.objects[i].value != int(i + 1)) return 5;
    return 0;
}
