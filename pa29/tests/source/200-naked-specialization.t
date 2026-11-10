template<class T> __attribute__((naked)) long specialized_raw(T) {
    asm("lea 9(%rdi),%rax; ret");
}
template<> long specialized_raw<int>(int value) { return value * 3; }
extern "C" long naked_specialization(long value) {
    return specialized_raw(value) + specialized_raw(5);
}
