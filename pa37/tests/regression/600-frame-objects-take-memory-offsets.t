// An access to a local object, or to a field of an object that a parameter
// promises readable, carries its offset in the memory immediate: the frame's
// address plus an offset in the frame does not wrap. A step that lost its
// allocation promise when the optimizer moved it stays in the storage that
// it started from.
struct Counter { int pad[5]; int hits; int limit; };
static int* kept;
extern "C" __attribute__((noinline)) void keep(int* p) { kept = p; }
extern "C" __attribute__((noinline)) int touch(int by) {
    *kept += by;
    return by + 1;
}
extern "C" __attribute__((noinline)) int locals_in_memory(int x, int n) {
    int a = x, b = 2 * x, c = 7;
    keep(&a);
    keep(&b);
    keep(&c);
    for (int i = 0; i < n; ++i) {
        a += touch(i);
        b ^= a;
        c += b;
    }
    return a + b + c;
}
extern "C" __attribute__((noinline)) int counted(Counter& counter, const int* values, int count) {
    int total = 0;
    for (int i = 0; i < count; ++i)
        if (values[i] > counter.limit) {
            ++counter.hits;
            total += values[i];
        }
    return total;
}
int main() {
    if (locals_in_memory(3, 4) != 45) return 1;
    Counter counter = {{0, 0, 0, 0, 0}, 0, 4};
    const int values[] = {1, 9, 4, 7, 5};
    if (counted(counter, values, 5) != 21) return 2;
    return counter.hits != 3;
}
