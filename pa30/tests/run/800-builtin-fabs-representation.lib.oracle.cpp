#include <cfenv>
#include <cstring>
extern "C" float fabs_single(float);
extern "C" double fabs_double(double);
extern "C" float fabs_derived_single(float);
extern "C" double fabs_derived_double(double);
extern "C" long double fabs_extended(long double);
extern "C" double fabs_once();
extern "C" double fabs_indirect(double);
extern "C" double fabs_converted(int*);
extern "C" double ordinary_fabs(double);
static int calls;
extern "C" double next_argument() { ++calls; return -2.0; }
int main() {
  const unsigned long long doubles[] = {
    0, 0x8000000000000000ULL, 0x3ff0000000000000ULL, 0xbff0000000000000ULL,
    0x7ff0000000000000ULL, 0xfff0000000000000ULL,
    0x7ff8000000000123ULL, 0xfff8000000000123ULL,
    0x7ff0000000000123ULL, 0xfff0000000000123ULL,
    1, 0x8000000000000001ULL, 0xfffffffffffffULL, 0x800fffffffffffffULL,
    0x10000000000000ULL, 0x8010000000000000ULL,
    0x7fefffffffffffffULL, 0xffefffffffffffffULL
  };
  for (auto bits : doubles) {
    double value; std::memcpy(&value, &bits, sizeof value);
    std::feclearexcept(FE_ALL_EXCEPT);
    value = fabs_double(value);
    unsigned long long result; std::memcpy(&result, &value, sizeof value);
    if (result != (bits & 0x7fffffffffffffffULL) || std::fetestexcept(FE_ALL_EXCEPT)) return 1;
  }
  const unsigned singles[] = {
    0, 0x80000000U, 0x3f800000U, 0xbf800000U, 0x7f800000U, 0xff800000U,
    0x7fc00123U, 0xffc00123U, 0x7f800123U, 0xff800123U,
    1, 0x80000001U, 0x7fffffU, 0x807fffffU, 0x800000U, 0x80800000U,
    0x7f7fffffU, 0xff7fffffU
  };
  for (auto bits : singles) {
    float value; std::memcpy(&value, &bits, sizeof value);
    std::feclearexcept(FE_ALL_EXCEPT);
    value = fabs_single(value);
    unsigned result; std::memcpy(&result, &value, sizeof value);
    if (result != (bits & 0x7fffffffU) || std::fetestexcept(FE_ALL_EXCEPT)) return 2;
  }
  int conversions = 0;
  if (fabs_derived_single(-0.5f) != 1.0f || fabs_derived_double(-0.5) != 1.0 ||
      fabs_extended(-3.5L) != 3.5L || fabs_once() != 2.0 || calls != 1 ||
      fabs_converted(&conversions) != 3.5 || conversions != 1 ||
      fabs_indirect(-4.5) != 4.5 || ordinary_fabs(-5.5) != -5.5) return 3;
}
