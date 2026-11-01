// 5.2.5p4, 14.8.2.1: a static member function named through an object
// deduces as the function itself, also from a class template's instance.
extern "C" int printf(const char*, ...);

template<class F> int by_value(F f) { return f(1, 2); }
template<class F> int by_lvalue(F& f) { return f(1, 2); }
template<class F> int by_forward(F&& f) { return f(3, 2); }
template<class... F> int count(F... f) { return sizeof...(f); }

struct S
{
	static bool less(int a, int b) { return a < b; }
};

template<class T> struct A
{
	static bool less(const T& a, const T& b) { return a < b; }
};

struct O
{
	static int g(int x) { return x + 1; }
	static int g(char c) { return c + 2; }
	template<class T> static T h(T t) { return t; }
};

int apply(int (*f)(int)) { return f(10); }
template<class T> T apply_explicit(T (*f)(T)) { return f(20); }

int main()
{
	S s;
	S* p = &s;
	A<int> x;
	O o;
	printf("%d %d %d %d\n", by_value(s.less), by_value(p->less), by_lvalue(s.less), by_forward(s.less));
	printf("%d %d %d\n", by_value(x.less), count(s.less, p->less), apply(o.h));
	return apply_explicit<int>(o.g) == 21 ? 0 : 1;
}
