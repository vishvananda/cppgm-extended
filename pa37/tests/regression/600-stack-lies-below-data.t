// The stack takes the low end of memory and the data follows it: the stack's
// low end is address 0, so a frame's test of its room compares the stack
// pointer alone, and a frame below the low end is outside memory.
static int cell = 7;
extern "C" __attribute__((noinline)) int below(int x) {
    volatile int local = x;
    volatile int* volatile on_stack = &local;
    int* volatile in_data = &cell;
    if ((unsigned long)on_stack < (unsigned long)in_data)
        return local + *in_data;
    return -1;
}
