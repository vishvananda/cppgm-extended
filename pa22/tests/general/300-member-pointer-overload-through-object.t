// 5.2.5p4: an object expression cannot name a non-static member as a value,
// so a pointer-to-member parameter is not viable for `s.m` and the static
// member's overload is selected.
struct S
{
	void m(int) {}
	static int m(char) { return 3; }
};

int pick(void (S::*)(int)) { return 1; }
int pick(int (*)(char)) { return 2; }

int main()
{
	S s;
	return pick(s.m) == 2 ? 0 : 1;
}
