// 5.2.5p4: a static member function named through an object is that
// function, an lvalue, so it converts and binds as the function does. The
// object expression is still evaluated once (9.4p2).
extern "C" int printf(const char*, ...);

struct S
{
	static bool less(int a, int b) { return a < b; }
};

struct O
{
	static int g(int x) { return x + 1; }
	static int g(char c) { return c + 2; }
};

int calls;
S object;
S& get()
{
	++calls;
	return object;
}

void take(bool (*p)(int, int)) { printf("%d\n", p(1, 2)); }
int take_int(int (*f)(int)) { return f(10); }

struct Wrap
{
	bool (*held)(int, int);
	Wrap(bool (*p)(int, int)) : held(p) {}
};
bool through(Wrap w) { return w.held(2, 1); }

int main()
{
	S s;
	S* p = &s;
	O o;
	bool (*f)(int, int) = s.less;
	bool (&r)(int, int) = p->less;
	printf("%d %d\n", f(1, 2), r(2, 1));
	f = &s.less;
	take(get().less);
	take(&get().less);
	printf("%d %d %d\n", calls, take_int(o.g), through(s.less));
	return f == S::less && (s.less)(1, 2) ? 0 : 1;
}
