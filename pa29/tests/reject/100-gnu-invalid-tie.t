int invalid(int value) {
    int result;
    asm("mov %1, %0" : "=r"(result) : "1"(value));
    return result;
}
