// The value and errno effects of the library are observable. Bit copies
// distinguish signed zero, and guarded paths include NaN and infinity.
extern "C" double sqrt(double);
extern "C" double fabs(double);
extern "C" double pow(double, double);
extern "C" int* __errno_location();
union Bits { double value; unsigned long long bits; };
__attribute__((noinline)) double root(double x) { return sqrt(x); }
__attribute__((noinline)) double length(double x, double y) { return sqrt(x*x + y*y); }
__attribute__((noinline)) double guarded(double x) { return x < 0 ? 0 : sqrt(x); }
__attribute__((noinline)) double square(double x) { return pow(x, 2); }
int main() {
    volatile double negative = -1, positive = 16;
    int* error = __errno_location(); *error = 0;
    double bad = root(negative);
    if (bad == bad || *error == 0) return 1;
    *error = 0;
    if (root(positive) != 4 || *error != 0) return 2;
    Bits z; z.bits = 0x8000000000000000ULL;
    Bits r; r.value = root(z.value); if (r.bits != z.bits) return 3;
    Bits nan; nan.bits = 0x7ff8000000000012ULL;
    double q = guarded(nan.value); if (q == q) return 4;
    Bits inf; inf.bits = 0x7ff0000000000000ULL;
    if (root(inf.value) != inf.value) return 5;
    if (guarded(negative) != 0 || length(3, 4) != 5) return 6;
    if (square(-3) != 9 || fabs(z.value) != 0) return 7;
    return 0;
}
