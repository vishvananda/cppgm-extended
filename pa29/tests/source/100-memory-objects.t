struct Block { unsigned low, high; };
extern "C" unsigned long read_block(const Block* block) {
    unsigned long result;
    asm("movq %1, %0" : "=r"(result) : "m"(*block));
    return result;
}
static unsigned& select(unsigned* object, unsigned* calls) {
    ++*calls;
    return *object;
}
extern "C" unsigned memory_update(unsigned input, unsigned* calls) {
    volatile unsigned value = input;
    asm volatile("addl $17, %0" : "+m"(value) : : "cc");
    unsigned result;
    asm("movl %1, %0" : "=r"(result) : "o"(value));
    asm("addl $7, %0" : "+m"(select(&result, calls)) : : "cc");
    return result;
}
