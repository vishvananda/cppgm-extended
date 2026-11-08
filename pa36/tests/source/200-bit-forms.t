extern "C" long asm_max(long a, long b) {
    asm("cmp %1, %0; cmovl %1, %0" : "+r"(a) : "r"(b) : "cc");
    return a;
}
extern "C" unsigned asm_less(long a, long b) {
    unsigned char result;
    asm("cmp %2, %1; setl %0" : "=qm"(result) : "r"(a), "r"(b) : "cc");
    return result;
}
extern "C" unsigned asm_bts(unsigned long* value, unsigned long index) {
    bool carry;
    asm("bts %2, %0" : "+m"(*value), "=@ccc"(carry) : "r"(index) : "memory");
    return carry;
}
extern "C" unsigned long asm_bswap(unsigned long value) {
    asm("bswap %0" : "+r"(value));
    return value;
}
extern "C" void asm_scans(unsigned long value, unsigned long* results) {
    unsigned long low, high;
    asm("bsf %2, %0; bsr %2, %1" : "=&r"(low), "=&r"(high) : "r"(value) : "cc");
    results[0] = low;
    results[1] = high;
}
extern "C" unsigned long asm_popcnt(unsigned long value) {
    unsigned long result;
    asm("popcnt %1, %0" : "=r"(result) : "r"(value) : "cc");
    return result;
}
extern "C" unsigned long asm_tzcnt(unsigned long value) {
    unsigned long result;
    asm("tzcnt %1, %0" : "=r"(result) : "r"(value) : "cc");
    return result;
}
extern "C" unsigned long asm_lzcnt(unsigned long value) {
    unsigned long result;
    asm("lzcnt %1, %0" : "=r"(result) : "r"(value) : "cc");
    return result;
}
extern "C" unsigned long asm_shld(unsigned long value, unsigned long other, unsigned char count) {
    asm("shld %b2, %1, %0" : "+r"(value) : "r"(other), "c"(count) : "cc");
    return value;
}
extern "C" unsigned long asm_shrd(unsigned long value, unsigned long other, unsigned char count) {
    asm("shrd %b2, %1, %0" : "+r"(value) : "r"(other), "c"(count) : "cc");
    return value;
}
extern "C" bool asm_cas128(unsigned __int128* pointer, unsigned long* expected,
    unsigned long low, unsigned long high) {
    unsigned long a = expected[0], d = expected[1];
    bool success;
    asm("lock cmpxchg16b %0" : "+m"(*pointer), "+a"(a), "+d"(d), "=@ccz"(success)
        : "b"(low), "c"(high) : "memory");
    expected[0] = a;
    expected[1] = d;
    return success;
}
extern "C" void asm_setmem(unsigned char* pointer, long a, long b) {
    asm("cmp %2, %1; setg %0" : "=m"(*pointer) : "r"(a), "r"(b) : "cc");
}
