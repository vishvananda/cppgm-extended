extern "C" void query(int leaf, unsigned* info) {
    __asm__ __volatile__("xchgq %%rbx, %q1;cpuid;xchgq %%rbx, %q1;"
        : "=a"(info[0]), "=r"(info[1]), "=c"(info[2]), "=d"(info[3])
        : "0"(leaf));
}
