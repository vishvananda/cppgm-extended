// 14.8.2.4p11: U stays without a value because no compared type uses it.
// The array bound N has its value from the other function's bound, so the
// first template is at least as general and the second is more specialized.
template<class U> struct X { typedef int t; };

template<class U = int, int N = 0> int f(int (&)[N], typename X<U>::t = 0) { return 1; }
template<class V = int> int f(int (&)[3], V* = 0) { return 2; }

int main()
{
	int a[3];
	return f(a) == 2 ? 0 : 1;
}
