// A function defined by the program retains its own semantics.
extern "C" __attribute__((noinline)) double pow(double x, double y) { return x + y + 17; }
extern "C" __attribute__((noinline)) double sqrt(double x) { return x + 29; }
int main() {
    volatile double x = 3;
    if (pow(x, 2) != 22 || sqrt(x) != 32) return 1;
    return 0;
}
