// N + 1 is a non-deduced context (14.8.2.5p5), but N has its value from the
// first parameter: nothing is left undeduced, so the second template is more
// specialized. The default template argument leaves V out of the ordering.
template<int N> int f(int (&)[N], int (*)[N + 1]) { return 1; }
template<class V = int> int f(int (&)[3], int (*)[4], V* = 0) { return 2; }

int main()
{
	int a[3];
	int b[4];
	return f(a, &b) == 2 ? 0 : 1;
}
