#include <cstdio>
extern "C" unsigned aligned_clobber(unsigned);
extern "C" unsigned aligned_exit(unsigned, int);
int main() {
    for (unsigned n = 0; n < 97; ++n) {
        if (aligned_clobber(n) != 2 * n + 1) return 1;
        if (aligned_exit(n, 0) != 2 * n + 5 || aligned_exit(n, 1) != 2 * n + 1) return 2;
    }
    std::puts("assembly clobbers and outgoing exits preserve over-aligned local storage");
}
