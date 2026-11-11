struct Words { unsigned long long low, high; };
constexpr long double precise = 0x1.0000000000000000000000001p0L;
constexpr long double delta = precise - 1.0L;
constexpr _Float64x alias = static_cast<_Float64x>(precise);
static_assert(alias > 1.0L, "target long-double representation alias");
constexpr long double tiny = 0x1p-16494L;
constexpr long double quiet = __builtin_nanl("0x1234");
constexpr long double infinity = __builtin_infl();
static_assert(precise > 1.0L, "target precision");
static_assert(delta * 0x1p100L == 1.0L, "constant arithmetic");
static_assert(7.0L / 2.0L == 3.5L && 9.0L * 0.25L == 2.25L, "arithmetic");
static_assert(static_cast<int>(-3.75L) == -3, "signed conversion");
static_assert(static_cast<double>(precise) == 1.0, "round to double");
static_assert(tiny > 0 && tiny / 2 == 0 && tiny * 2 == 0x1p-16493L, "subnormals");
static_assert(__builtin_isnan(quiet) && !__builtin_isfinite(infinity), "special values");
static_assert(__builtin_signbit(-0.0L) && !__builtin_isnormal(tiny), "classification");
static_assert(__builtin_isunordered(quiet, 0.0L) && !__builtin_isgreater(quiet, 0.0L), "quiet comparison");
constexpr long double tie = 1.0L + 0x1p-53L;
static_assert(static_cast<double>(tie) == 1.0, "ties to even");
static_assert(static_cast<double>(tie + 0x1p-100L) > 1.0, "single target rounding");
constexpr __int128 wide = static_cast<__int128>(0x1p100L);
static_assert(static_cast<long double>(wide) == 0x1p100L, "wide integer conversion");
constexpr long double exact_list{wide};
static_assert(exact_list == 0x1p100L, "wide integer list initialization");
constexpr unsigned long long largest = static_cast<unsigned long long>(0x1p64L - 1);
static_assert(largest == ~0ULL, "unsigned conversion");
constexpr int bound = static_cast<int>(delta * 0x1p100L);
char bounded[bound + 2];
static_assert(sizeof(bounded) == 3, "target value in an array bound");
constexpr Words bits = __builtin_bit_cast(Words, precise);
static_assert(bits.low == 0x1000 && bits.high == 0x3fff000000000000ULL, "encoded bits");
constexpr int step() {
 long double value = 1;
 for (int i = 0; i < 3; ++i) { value += 2; --value; }
 return static_cast<int>(value);
}
static_assert(step() == 4, "compound operations");
constexpr long double operator""_quad(long double value) { return value; }
static_assert(0x1.0000000000000000000000001p0_quad > 1.0L, "literal argument precision");
extern "C" int observe() {
 unsigned long long words[2] = {};
 __builtin_memcpy(words, &precise, 16);
 return words[0] == 0x1000 && words[1] == 0x3fff000000000000ULL && step() == 4 ? 17 : 2;
}
