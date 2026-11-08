#define MASK_TARGET __attribute__((target("avx512f,avx512bw,avx512dq")))
extern "C" MASK_TARGET void mask_8(unsigned char value, unsigned char* output) {
    unsigned char result;
    asm volatile("kmovb %1,%0" : "=&Yk"(result) : "k"(value) : "k6");
    *output = result;
}
extern "C" MASK_TARGET unsigned char mask_tied_8(unsigned char value) {
    asm("kmovb %0,%0" : "+k"(value));
    return value;
}
extern "C" MASK_TARGET void mask_16(unsigned short value, unsigned short* output) {
    unsigned short result;
    asm volatile("kmovw %1,%0" : "=&Yk"(result) : "k"(value) : "k6");
    *output = result;
}
extern "C" MASK_TARGET unsigned short mask_tied_16(unsigned short value) {
    asm("kmovw %0,%0" : "+k"(value));
    return value;
}
extern "C" MASK_TARGET void mask_32(unsigned value, unsigned* output) {
    unsigned result;
    asm volatile("kmovd %1,%0" : "=&Yk"(result) : "k"(value) : "k6");
    *output = result;
}
extern "C" MASK_TARGET unsigned mask_tied_32(unsigned value) {
    asm("kmovd %0,%0" : "+k"(value));
    return value;
}
extern "C" MASK_TARGET void mask_64(unsigned long long value, unsigned long long* output) {
    unsigned long long result;
    asm volatile("kmovq %1,%0" : "=&Yk"(result) : "k"(value) : "k6");
    *output = result;
}
extern "C" MASK_TARGET unsigned long long mask_tied_64(unsigned long long value) {
    asm("kmovq %0,%0" : "+k"(value));
    return value;
}
