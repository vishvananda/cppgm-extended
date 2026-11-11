extern "C" long double echo_quad(long double);
extern "C" int observe() {
 long double value = 1.5L;
 long double result = echo_quad(value);
 unsigned long long bits[2] = {};
 __builtin_memcpy(bits, &result, 16);
 return bits[0] == 0 && bits[1] == 0x3fff800000000000ULL ? 1 : 0;
}
extern "C" int observe_precise() {
 long double value = 0x1.0000000000000000000000001p0L;
 long double result = echo_quad(value);
 unsigned long long bits[2] = {};
 __builtin_memcpy(bits, &result, 16);
 return bits[0] == 0x1000 && bits[1] == 0x3fff000000000000ULL ? 1 : 0;
}
