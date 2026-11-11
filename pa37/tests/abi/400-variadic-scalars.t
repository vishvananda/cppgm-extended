extern "C" long long scalar_sum(int, double, ...);
extern "C" int empty_arguments(int, ...);
typedef long long (*Call)(int, double, ...);
Call volatile saved = scalar_sum;
extern "C" int observe() {
 int pointed = 42;
 signed char last = -7;
 float floating = 2.5f;
 long long result = saved(5, 1.5, 0x12345678, floating, 0x100000002LL, &pointed, last);
 return result == 4600387242LL && empty_arguments(9) == 9 ? 17 : 1;
}
