// Calls on both virtual targets and through function pointers keep their
// results, replacement boundaries and cleanups on normal and throwing paths.
static int live, destroyed;
struct Guard { Guard() { ++live; } ~Guard() { --live; ++destroyed; } };
struct Base { virtual int value(int x) { return x + 1; } virtual ~Base() {} };
struct Derived : Base { int value(int x) { return x + 7; } };
__attribute__((noinline)) int dispatch(Base& b, int x) { Guard g; return b.value(x); }
int add(int x) { return x + 3; }
int twice(int x) { return x * 2; }
__attribute__((noinline)) int through(int (*f)(int), int x) { return f(x); }
__attribute__((noinline)) int throwing(int x) { Guard g; if (x < 0) throw x; return x + 5; }
__attribute__((noinline)) int recursion(int x) { return x > 0 ? x + recursion(x - 1) : 0; }
int main() {
    volatile int input = 9;
    Base b; Derived d;
    if (dispatch(b, input) != 10 || dispatch(d, input) != 16 || live) return 1;
    if (through(add, input) != 12 || through(twice, input) != 18) return 2;
    if (throwing(input) != 14 || live) return 3;
    try { throwing(-input); return 4; } catch (int n) { if (n != -9 || live) return 5; }
    if (destroyed != 4 || recursion(input) != 45) return 6;
    return 0;
}
