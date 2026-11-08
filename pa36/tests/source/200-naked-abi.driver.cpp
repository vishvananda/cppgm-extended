#include <cstdio>
extern "C" long naked_sum(long, long);
extern "C" long naked_local(long);
extern "C" long naked_callback(long (*)(long), long);
extern "C" long naked_tail(long (*)(long), long);
extern "C" long naked_same_unit(long, long);
static long callback(long value) { return value * 7 - 2; }
int main() {
    for (long a = -31; a < 32; ++a) {
        if (naked_local(a) != a * 3 + 3) return 1;
        if (naked_callback(callback, a) != callback(a) + 5) return 2;
        if (naked_tail(callback, a) != callback(a)) return 3;
        for (long b = -13; b < 14; ++b) {
            if (naked_sum(a, b) != a + b * 2) return 4;
            if (naked_same_unit(a, b) != (a + b) * 3 + 11) return 5;
        }
    }
    std::puts("naked ABI, local labels, indirect calls and normal returns passed");
}
