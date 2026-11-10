#include <cstdio>
extern "C" void reserved_output(unsigned long*);
int main() {
    unsigned long values[3] = {};
    reserved_output(values);
    if (values[0] != 123 || values[1] != 1 || values[2] != 97) return 1;
    std::puts("reserved-register outputs are captured before frame-base restoration, preserving flags");
}
