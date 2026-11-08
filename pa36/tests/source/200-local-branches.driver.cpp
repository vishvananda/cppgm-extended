#include <cstdio>
extern "C" unsigned branch_sum(unsigned, unsigned);
int main() {
    for (unsigned a = 0; a < 32; ++a)
        for (unsigned b = 0; b < 32; ++b)
            if (branch_sum(a, b) != (a * (a + 1) + b * (b + 1)) / 2) return 1;
    std::puts("numeric, named and unique local branches survive inlining and cloning");
}
