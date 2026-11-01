// A GNU vector of 16 bytes is a v128 value and its operators and SSE builtins
// are vector instructions, and a scalar operand spreads to every lane. A vector
// of another size stays lane by lane, and so does an operation SSE2 lacks.
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef unsigned v4su __attribute__((vector_size(16)));
typedef int v2si __attribute__((vector_size(8)));

v4sf add_floats(v4sf a, v4sf b) { return a + b; }
v2df divide_doubles(v2df a, v2df b) { return a / b; }
v4si add_words(v4si a, v4si b) { return a + b; }
v4si multiply_words(v4si a, v4si b) { return a * b; }
v4si bit_mix(v4si a, v4si b) { return (a & b) ^ b; }
v4si compare_words(v4si a, v4si b) { return a == b; }
v4si shift_left(v4si a) { return a << 3; }
v4si shift_right(v4si a, int count) { return a >> count; }
v4si shift_right_literal(v4si a) { return a >> 2; }
v4su shift_unsigned(v4su a, int count) { return a >> count; }
v8hi shift_short(v8hi a, int count) { return a << count; }
v2di shift_quad(v2di a, int count) { return a >> count; }
v4sf scale_floats(v4sf a, float b) { return a * b; }
v2df scale_doubles(v2df a, double b) { return b * a; }
v4si add_scalar(v4si a, int b) { return a + b; }
v2di add_quad_scalar(v2di a, long long b) { return a + b; }
v4sf compare_scalar(v4sf a, float b) { return (v4sf)(a < b); }
v4si compare_zero(v4si a) { return a > 0; }
v2df pair(double a, double b) { v2df r = {a, b}; return r; }
v4si words(int a, int b, int c, int d) { v4si r = {a, b, c, d}; return r; }
double second_lane(v2df a) { return a[1]; }
float third_lane(v4sf a) { return a[2]; }
int first_word(v4si a) { return a[0]; }
int sign_mask(v4sf a) { return __builtin_ia32_movmskps(a); }
v2df add_low(v2df a, v2df b) { return __builtin_ia32_addsd(a, b); }
v4sf root(v4sf a) { return __builtin_ia32_sqrtps(a); }
v4si shuffle_words(v4si a) { return __builtin_ia32_pshufd(a, 27); }
v2si small_add(v2si a, v2si b) { return a + b; }

v2df store_load(const v2df* p, v2df* q)
{
	v2df value = *p;
	*q = value;
	return value;
}

int main() { return 0; }
