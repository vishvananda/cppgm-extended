extern "C" long asm_s8(signed char value) {
    long result;
    asm("movsbq %1, %0" : "=r"(result) : "q"(value));
    return result;
}
extern "C" unsigned long asm_u8(unsigned char value) {
    unsigned long result;
    asm("movzbq %1, %0" : "=r"(result) : "q"(value));
    return result;
}
extern "C" long asm_s16(short value) {
    long result;
    asm("movswq %1, %0" : "=r"(result) : "r"(value));
    return result;
}
extern "C" unsigned long asm_u16(unsigned short value) {
    unsigned long result;
    asm("movzwq %1, %0" : "=r"(result) : "r"(value));
    return result;
}
extern "C" long asm_s32(int value) {
    long result;
    asm("movslq %1, %0" : "=r"(result) : "r"(value));
    return result;
}
extern "C" unsigned asm_high(unsigned short value) {
    unsigned result;
    asm("movzbl %h1, %0" : "=Q"(result) : "a"(value));
    return result;
}
extern "C" short asm_s8_16(signed char value) {
    short result;
    asm("movsbw %1, %0" : "=r"(result) : "q"(value));
    return result;
}
extern "C" long asm_memory_s8(const signed char* pointer) {
    long result;
    asm("movsbq %1, %0" : "=r"(result) : "m"(*pointer));
    return result;
}
