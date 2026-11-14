// A frame is tested against the low end of the stack once, before the stack
// pointer moves, and a frame that does not fit there traps. A function that
// calls nothing leaves the stack pointer where it found it.
extern "C" __attribute__((noinline)) int leaf(int x) {
    volatile int cells[4];
    cells[0] = x;
    cells[1] = x + 1;
    cells[2] = cells[0] + cells[1];
    cells[3] = cells[2] * 2;
    return cells[3] + cells[1];
}
extern "C" __attribute__((noinline)) int descend(int depth) {
    volatile char block[4096];
    block[0] = (char)depth;
    block[4095] = 1;
    if (depth == 0)
        return block[0] + block[4095];
    return descend(depth - 1) + block[4095];
}
