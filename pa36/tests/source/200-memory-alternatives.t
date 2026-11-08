extern "C" unsigned mixed(unsigned value) {
    unsigned result;
    asm("movl %1, %0; addl $3, %0" : "=&r"(result) : "rm"(value) : "cc");
    asm("addl %1, %0" : "+m,r"(result) : "i,i"(5) : "cc");
    asm("addl %1, %0" : "+g"(result) : "g"(7) : "cc");
    return result;
}
