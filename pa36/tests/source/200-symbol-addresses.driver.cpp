#include <cstdio>
extern "C" {
int assembly_values[5] = {11, 23, 37, 41, 59};
int* assembly_address();
int* assembly_offset();
int assembly_read();
}
int main() {
    if (assembly_address() != assembly_values || assembly_offset() != assembly_values + 3) return 1;
    if (assembly_read() != 23) return 2;
    assembly_values[1] = 71;
    if (assembly_read() != 71) return 3;
    std::puts("symbol identities and byte addends survive source and LowIR roundtrips");
}
