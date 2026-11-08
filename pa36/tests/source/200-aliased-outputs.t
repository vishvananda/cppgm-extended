extern "C" void alias(void* address) {
    unsigned long long* output = static_cast<unsigned long long*>(address);
    asm("movabsq $0x1122334455667788, %%rax; movabsq $0x99aabbccddeeff00, %%rcx"
        : "=a"(*output), "=c"(*output));
}
