// 14.8.2.4p11: a template parameter may stay without a value only where it
// is not used in the types being ordered, so a parameter only a non-deduced
// context names makes its template more specialized than one deducing any
// type there; the type the two call on stays deduced elsewhere.
template<class T> struct id { typedef T type; };
template<class T> struct always { typedef int type; };
template<bool B, class T> struct enable { };
template<class T> struct enable<true, T> { typedef T type; };

template<class T, class I> int p(I) { return 1; }
template<class T> int p(typename id<T>::type) { return 2; }

template<class T, class I> int q(I, int) { return 1; }
template<class T> int q(typename enable<sizeof(T) == 4, T>::type, int) { return 2; }

template<class T, class I> int u(I) { return 1; }
template<class T> int u(typename always<T>::type) { return 2; }

template<class T> using alias = typename id<T>::type;
template<class T, class I> int v(I) { return 1; }
template<class T> int v(alias<T>) { return 2; }

template<class T> int r(T, typename id<T>::type) { return 1; }
template<class T> int r(T*, typename id<T>::type) { return 2; }

template<class T, class U = int> int t(T) { return 1; }
template<class T> int t(T*) { return 2; }

int main()
{
	int x = 0;
	return (p<int>(x) == 2 && q<int>(x, 1) == 2 && u<char>(3) == 2 && v<int>(x) == 2 && r(&x, &x) == 1 && t(&x) == 2)
		? 0
		: 1;
}
