#include <cstdio>
extern "C" void commutative_constant(unsigned long, unsigned long*);
extern "C" void commutative_storage(unsigned long, unsigned long*, unsigned long*);
int main() {
    for (unsigned long n = 0; n < 97; ++n) {
        unsigned long out = 0, source = n * 13;
        commutative_constant(n, &out);
        if (out != n + 5) return 1;
        commutative_storage(n, &source, &out);
        if (out != n + source) return 2;
    }
    std::puts("commutative swaps preserve evaluated values and memory identities");
}
