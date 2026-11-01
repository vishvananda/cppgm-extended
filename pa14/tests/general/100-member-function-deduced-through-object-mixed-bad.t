// An overload set gives F no single type: two members match, so the call
// has no deduced argument (14.8.2.1p6).
struct S
{
	void m(int) {}
	static void m(char) {}
};

template<class F> void take(F) {}

int main()
{
	S s;
	take(s.m);
}
