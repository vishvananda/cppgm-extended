// Deduction tries each member of the set (14.8.2.1p6): only the static one
// has a `char` parameter, and that is what the call selects (5.2.5p4).
struct S
{
	void m(int) {}
	static int m(char) { return 7; }
};

template<class R> int take(R (*f)(char)) { return f('a') == 7 ? 0 : 1; }

int main()
{
	S s;
	return take(s.m);
}
