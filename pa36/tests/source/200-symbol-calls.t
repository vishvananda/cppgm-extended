extern "C" unsigned long assembly_callback();
extern "C" void assembly_prepare();
extern "C" unsigned long assembly_call() {
    assembly_prepare();
    unsigned long result;
    asm volatile("call %P1" : "=a"(result) : "X"(&assembly_callback)
        : "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc");
    return result;
}
