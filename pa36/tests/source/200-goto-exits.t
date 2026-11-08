extern "C" int goto_select(int value) {
    asm goto("test %0, %0; js %l[negative]; je %l[zero]"
        : : "r"(value) : "cc" : negative, zero);
    return 7;
zero:
    return 8;
negative:
    return 9;
}
extern "C" int goto_output(int value) {
    asm goto("sub $1, %0; js %l2" : "+r"(value) : : "cc" : negative);
    return value * 3;
negative:
    return value * 5;
}
extern "C" int goto_loop(int count) {
    int sum = 0;
again:
    asm goto("test %1, %1; je %l[done]; add %1, %0; sub $1, %1; jmp %l[again]"
        : "+r"(sum), "+r"(count) : : "cc" : done, again);
done:
    return sum;
}
