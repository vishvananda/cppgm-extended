extern "C" unsigned aligned_clobber(unsigned value) {
    alignas(64) volatile unsigned values[32];
    values[0] = value;
    values[31] = value + 1;
    asm volatile("mov $0, %%r12" : : : "r12", "memory");
    return values[0] + values[31];
}
extern "C" unsigned aligned_exit(unsigned value, int leave) {
    alignas(64) volatile unsigned values[32];
    values[0] = value;
    values[31] = value + 1;
    asm goto("mov $0, %%r12; test %0, %0; jne %l[done]" : : "r"(leave) : "r12", "memory", "cc" : done);
    values[0] += 4;
done:
    return values[0] + values[31];
}
