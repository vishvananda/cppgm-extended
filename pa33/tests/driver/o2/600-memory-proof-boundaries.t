// Disjoint fields can reuse values. Aliases, bytes, unions, escaped locals
// and calls that change storage must still observe the new value.
extern "C" void* malloc(unsigned long);
extern "C" void free(void*);
struct Cell { int value, other; };
static int* escaped;
__attribute__((noinline)) int fields(Cell* a, Cell* b) {
    int v = a->value; b->other = v + 3; return a->value + v;
}
__attribute__((noinline)) int aliases(int* a, int* b) {
    int v = *a; *b = v + 3; return *a + v;
}
__attribute__((noinline)) unsigned bytes(unsigned* p, unsigned char* q) {
    unsigned a = *p; *q = 0; return a ^ *p;
}
__attribute__((noinline)) void change() { *escaped = 11; }
__attribute__((noinline)) int call_path(int v) {
    int local = v; escaped = &local; change(); escaped = 0; return local;
}
__attribute__((noinline)) int private_slot(int* out, int v) {
    int local = v; *out = 9; return local;
}
__attribute__((noinline)) int allocated(int* existing, int v) {
    int* fresh = static_cast<int*>(malloc(sizeof(int)));
    if (!fresh) return *existing;
    *fresh = 91; int result = *existing; free(fresh); return result;
}
__attribute__((noinline)) int zero_trip(const int* p, unsigned n) {
    int sum = 0; for (unsigned i = 0; i < n; ++i) sum += *p; return sum;
}
int main() {
    volatile int input = 7;
    Cell c = {input, 0}, d = {0, 0};
    if (fields(&c, &c) != 14 || c.other != 10) return 1;
    if (fields(&c, &d) != 14 || d.other != 10) return 2;
    int x = input, y = 1;
    if (aliases(&x, &x) != 17 || x != 10) return 3;
    x = input; if (aliases(&x, &y) != 14 || y != 10) return 4;
    unsigned bits = 0x12345678u;
    if (bytes(&bits, reinterpret_cast<unsigned char*>(&bits)) != 0x78u) return 5;
    if (call_path(input) != 11 || private_slot(&x, input) != 7) return 6;
    if (allocated(&y, input) != 10) return 7;
    if (zero_trip(0, 0) != 0 || zero_trip(&x, 3) != 27) return 8;
    union U { unsigned value; unsigned char byte[4]; } u = {0xabcdef12u};
    u.byte[0] = 0; if (u.value != 0xabcdef00u) return 9;
    return 0;
}
