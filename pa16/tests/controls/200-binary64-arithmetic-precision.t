// expect exit 0
// PA16: constant evaluation, static initialization and execution use the
// same binary64 precision. Extra intermediate precision double-rounds these
// additions, subtractions, products and quotients to a neighboring value.
constexpr double delta = 1.1104940751682779e-16;
constexpr double product_left = -1.3290626730907361e20;
constexpr double product_right = -1.0246854044092082e101;
constexpr double quotient_left = 1.3608547793652755e-308;
constexpr double quotient_right = 2.765960442749773e-83;
constexpr double sum = 1.0 + delta;
constexpr double difference = 1.0 - -delta;
constexpr double product = product_left * product_right;
constexpr double quotient = quotient_left / quotient_right;
static_assert(sum == 1.0000000000000002, "binary64 addition");
static_assert(difference == 1.0000000000000002, "binary64 subtraction");
static_assert(product == 1.3618711226611643e121, "binary64 multiplication");
static_assert(quotient == 4.9200081039929301e-226, "binary64 division");
double static_sum = 1.0 + delta;
double static_difference = 1.0 - -delta;
double static_product = product_left * product_right;
double static_quotient = quotient_left / quotient_right;
constexpr long double extended_delta = 8.67361737988403547205962240695953369140625e-19L;
constexpr long double extended = (1.0L + extended_delta) - 1.0L;
static_assert(extended == extended_delta, "long double retains extended precision");
int main() {
  volatile double one = 1.0, small = delta, negative_small = -delta;
  volatile double a = product_left, b = product_right;
  volatile double c = quotient_left, d = quotient_right;
  volatile long double wide_one = 1.0L, wide_small = extended_delta;
  return static_sum != sum || static_difference != difference ||
         static_product != product || static_quotient != quotient ||
         one + small != sum || one - negative_small != difference ||
         a * b != product || c / d != quotient ||
         (wide_one + wide_small) - wide_one != extended;
}
