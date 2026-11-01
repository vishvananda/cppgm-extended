// A pointer-to-member target selects the non-static member, which an object
// expression cannot name as a value (5.2.5p4); 5.3.1p4 requires `&S::m`.
struct S
{
	void m(int) {}
	static void m(char) {}
};

void take(void (S::*)(int)) {}

int main()
{
	S s;
	take(s.m);
}
