#include <cstdio>
extern "C" int bitfield_conditional_places(int);
int main() {
    for (int i = 0; i < 2; ++i) {
        int result = bitfield_conditional_places(i);
        if (result) { std::printf("case %d failed at %d\n", i, result); return result; }
    }
    std::puts("bit-field outputs preserve neighbors and evaluate selected places once");
}
