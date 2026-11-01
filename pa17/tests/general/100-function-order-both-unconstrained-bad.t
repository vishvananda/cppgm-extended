// 14.8.2.4p11: T is not used in the parameter type of the first template,
// and in the second it is deduced: neither is more specialized.
template<class T, class I> int p(I) { return 1; }
template<class T> int p(T) { return 2; }

int main()
{
	return p<int>(0);
}
