constexpr long double operator""_value(long double n) { return n; }
constexpr unsigned operator""_raw(const char *p) { return p[1] == '\'' ? 17 : 0; }
template<char... Chars> constexpr unsigned operator""_count() { return sizeof...(Chars); }
static_assert(2'62.1'44_value == 262.144L, "cooked constant value");
static_assert(2'62.1'44_raw == 17, "raw constant spelling");
static_assert(2'62.1'44_count == 9, "template constant spelling");
static unsigned calls;
long double operator""_plus(long double n) { ++calls; return n + 1; }
long double global = 1'0.0_plus;
__attribute__((noinline)) long double value() { return 2'62.1'44_value; }
int main() {
    volatile double d = 262'144.0;
    volatile float f = 1'2.5e+1'0f;
    volatile long double x = 0x1'f.8p+1L;
    volatile float tiny = 1'0e-4'0f;
    if (global != 11.0L || calls != 1) return 3;
    if (d != 262144.0 || f != 125000000000.0f || x != 63.0L || tiny != 1e-39f) return 1;
    if (value() != 262.144L || 2'62.1'44_raw != 17 || 2'62.1'44_count != 9) return 2;
    return 0;
}
