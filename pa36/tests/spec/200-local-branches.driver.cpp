#include <cstdio>
extern "C" void direct_branch(unsigned long, unsigned long*);
int main() {
    for (unsigned long n = 0; n < 50; ++n) {
        unsigned long value = 0;
        direct_branch(n, &value);
        if (value != n * (n + 1) / 2) return 1;
    }
    std::puts("frontend-neutral Intel local branches and separate label namespaces passed");
}
