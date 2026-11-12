#include <limits>
#include <cwchar>
#include <memory>
#include <tuple>
#include <utility>
static_assert(std::numeric_limits<unsigned long>::digits == 32, "target integer limits");
static_assert(std::numeric_limits<float>::digits == 24, "binary32");
static_assert(std::numeric_limits<double>::digits == 53, "binary64");
static_assert(std::numeric_limits<long double>::digits == 113, "binary128");
static_assert(std::numeric_limits<long double>::max_exponent == 16384, "target exponent");
static_assert(__is_same(std::wint_t, int), "target wide-character type");
extern "C" int observe() {
 std::unique_ptr<int> owned(new int(17));
 if (*owned != 17) return 4;
 std::tuple<int, unsigned long> pair = std::make_tuple(3, 14UL);
 if (!(std::numeric_limits<long double>::min() > 0.0L)) return 1;
 if (!(std::numeric_limits<long double>::max() > 1e4000L)) return 2;
 if (!(std::numeric_limits<long double>::epsilon() < 1e-33L)) return 3;
 return std::get<0>(pair) + std::get<1>(pair);
}
