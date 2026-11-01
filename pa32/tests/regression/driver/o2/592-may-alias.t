// An access through a may_alias type may read any object: the load of *f
// after each write stays. Without the attribute the second load folds.
typedef int __attribute__((__may_alias__)) any_int;
typedef long any_long __attribute__((may_alias));
struct __attribute__((may_alias)) Leading { int bits; };
struct Trailing { int bits; } __attribute__((__may_alias__));
typedef struct { int bits; } __attribute__((may_alias)) Named;
struct Plain { int bits; };
typedef Plain __attribute__((may_alias)) Later;

__attribute__((noinline)) float through_typedef(float* f, any_int* i)
{
	float a = *f;
	*i = 1;
	return a + *f;
}

__attribute__((noinline)) double through_declarator_typedef(double* f, any_long* i)
{
	double a = *f;
	*i = 1;
	return a + *f;
}

__attribute__((noinline)) float through_leading(float* f, Leading* i)
{
	float a = *f;
	i->bits = 1;
	return a + *f;
}

__attribute__((noinline)) float through_trailing(float* f, Trailing* i)
{
	float a = *f;
	i->bits = 1;
	return a + *f;
}

__attribute__((noinline)) float through_named(float* f, Named* i)
{
	float a = *f;
	i->bits = 1;
	return a + *f;
}

__attribute__((noinline)) float through_later_typedef(float* f, Later* i)
{
	float a = *f;
	i->bits = 1;
	return a + *f;
}

// short has no may_alias type: its load folds.
__attribute__((noinline)) short plain(short* f, float* i)
{
	short a = *f;
	*i = 1;
	return a + *f;
}

int main()
{
	return 0;
}
