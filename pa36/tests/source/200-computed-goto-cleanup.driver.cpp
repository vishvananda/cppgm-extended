#include <cstdio>
extern "C" int computed_cleanup();
extern "C" int direct_cleanup();
int main() {
    if (computed_cleanup() != 0 || direct_cleanup() != 1) return 1;
    std::puts("GNU computed goto skips cleanup; direct goto destroys");
}
