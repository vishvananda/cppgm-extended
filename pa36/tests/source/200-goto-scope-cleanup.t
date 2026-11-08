// Check scope-exit cleanup directly; the host GCC drops it on an asm-goto edge.
struct Guard {
    int* log;
    Guard(int* p) : log(p) { *log += 1; }
    ~Guard() { *log += 10; }
};
extern "C" int goto_cleanup(int leave) {
    int log = 0;
    {
        Guard guard(&log);
        asm goto("test %0, %0; jne %l[out]" : : "r"(leave) : "cc" : out);
        log += 100;
    }
    return log;
out:
    return log;
}
