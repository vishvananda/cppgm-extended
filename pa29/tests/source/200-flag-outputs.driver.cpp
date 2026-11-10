#include <climits>
#include <cstdio>
extern "C" unsigned flags(long, long);
int main() {
    const long values[] = {LONG_MIN, -129, -1, 0, 1, 127, LONG_MAX};
    for (long left : values) for (long right : values) {
        const unsigned expected = static_cast<unsigned long>(left) < static_cast<unsigned long>(right)
            ? 1 : left == right ? 2 : 0;
        if (flags(left, right) != expected) return 1;
    }
    std::puts("flag outputs: carry and zero passed");
}
