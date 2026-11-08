struct Fields { unsigned low : 5; signed middle : 7; unsigned high : 20; };
extern "C" int bitfield_places(int choose) {
    Fields p = {3, -5, 0xabcde};
    asm("mov $27, %0" : "=r"(p.low));
    if (p.low != 27 || p.middle != -5 || p.high != 0xabcde) return 1;
    asm("add $9, %0" : "+r"(p.low) : : "cc");
    if (p.low != 4 || p.middle != -5 || p.high != 0xabcde) return 2;
    asm("mov $-41, %0" : "=r"(p.middle));
    if (p.middle != -41 || p.low != 4 || p.high != 0xabcde) return 3;
    int calls = 0;
    asm("add $2, %0" : "+r"((++calls, ++p.low)) : : "cc");
    if (calls != 1 || p.low != 7u) return 5;
    asm("add $2, %0" : "+r"((++calls, p.low = 12)) : : "cc");
    if (calls != 2 || p.low != 14 || p.middle != -41 || p.high != 0xabcde) return 6;
    volatile Fields v = {1, -3, 0x12345};
    asm("mov $9, %0; mov $-7, %1" : "=r"(v.low), "=r"(v.middle));
    if (v.low != 9 || v.middle != -7 || v.high != 0x12345) return 7;
    asm("add $1, %0" : "+r"(v.low) : : "cc");
    return v.low == 10 && v.middle == -7 && v.high == 0x12345 ? 0 : 8;
}
