#include <cstdio>
extern "C" void first(unsigned long*);
extern "C" void second(unsigned long*);
extern "C" void both(unsigned long*, unsigned long*);
int main() {
    unsigned long a = 0, b = 0, c = 0, d = 0;
    first(&a);
    second(&b);
    both(&c, &d);
    if (a != 7 || b != 13 || c != 23 || d != 31) return 1;
    std::puts("one template with distinct invocation constants passed");
}
