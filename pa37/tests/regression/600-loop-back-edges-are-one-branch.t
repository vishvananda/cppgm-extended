// A loop's back edge is one `br_if`: the moves of the loop's own values run
// before it when nothing outside the loop reads those values. A value that is
// read after the loop keeps its moves on the taken side. A test of a
// comparison's result reads the result as it is.
extern "C" __attribute__((noinline)) unsigned mixed(const unsigned* values, unsigned count, unsigned seed) {
    for (unsigned i = 0; i < count; ++i) seed = (seed ^ values[i]) * 16777619u + i;
    return seed;
}
extern "C" __attribute__((noinline)) unsigned find(const unsigned char* text, unsigned size, unsigned stop) {
    unsigned at = 0;
    while (at < size && text[at] != stop) ++at;
    return at;
}
extern "C" __attribute__((noinline)) unsigned larger(unsigned a, unsigned b, unsigned c) {
    if (a < b) return c + 1;
    return c - 1;
}
int main() {
    const unsigned values[] = {3, 1, 4, 1, 5};
    if (mixed(values, 5, 7) != 525490795u) return 1;
    if (mixed(values, 0, 7) != 7) return 2;
    const unsigned char text[] = "abc;def";
    if (find(text, 7, ';') != 3) return 3;
    if (find(text, 7, '!') != 7) return 4;
    return larger(1, 2, 10) != 11 || larger(2, 1, 10) != 9;
}
