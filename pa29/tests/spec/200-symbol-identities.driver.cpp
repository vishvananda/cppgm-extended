#include <cstdio>
extern "C" {
extern long assembly_left, assembly_right;
long assembly_external = 73;
void direct_symbols(long**, long**, long**, long**);
}
int main() {
    for (unsigned i = 0; i < 17; ++i) {
        long *a, *b, *c, *d;
        direct_symbols(&a, &b, &c, &d);
        if (a != &assembly_left || b != &assembly_right || c != &assembly_left + 1 ||
            d != &assembly_external || *a != 17 || *b != 29 || *d != 73) return 1;
    }
    std::puts("per-invocation symbol identities, scaled addends and GOT references passed");
}
