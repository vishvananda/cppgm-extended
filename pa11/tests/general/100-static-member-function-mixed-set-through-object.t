// 5.2.5p4: an overloaded member named through an object is what overload
// resolution selects. A target of function type selects the static member,
// which needs no object; the object expression is still evaluated.
struct S
{
	void m(int) {}
	static int m(char) { return 3; }
};

int count = 0;
S object;

S& side()
{
	++count;
	return object;
}

int call(int (*f)(char)) { return f('a'); }

int main()
{
	S s;
	int (*p)(char) = side().m;
	int (&r)(char) = s.m;
	return p('a') == 3 && r('b') == 3 && call(s.m) == 3 && count == 1 ? 0 : 1;
}
