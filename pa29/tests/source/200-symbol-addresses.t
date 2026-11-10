// Hidden data permits literal symbol constraints with both host compilers under PIC.
extern "C" __attribute__((visibility("hidden"))) int assembly_values[5];
extern "C" int* assembly_address() {
    int* result;
    asm("lea %c1(%%rip), %0" : "=r"(result) : "i"(&assembly_values[0]));
    return result;
}
extern "C" int* assembly_offset() {
    int* result;
    asm("lea %c1+4(%%rip), %0" : "=r"(result) : "i"(&assembly_values[2]));
    return result;
}
extern "C" int assembly_read() {
    int result;
    asm("mov %c1(%%rip), %0" : "=r"(result) : "i"(&assembly_values[1]) : "memory");
    return result;
}
