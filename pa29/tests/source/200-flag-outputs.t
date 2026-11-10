extern "C" unsigned flags(long left, long right) {
    bool carry, zero;
    asm("cmp %3, %2" : "=@ccb"(carry), "=@cce"(zero) : "r"(left), "r"(right) : "cc");
    return static_cast<unsigned>(carry) + 2 * static_cast<unsigned>(zero);
}
