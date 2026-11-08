// CXX11-124: equivalent sizeof operands must retain captured alias arguments.
// N3485 5.3.3p5,14.5.7p2; expect exit 0
template<unsigned long N> struct Size {};
template<class... Q> using Count = Size<sizeof...(Q)>;
template<class... P> void prior(Size<sizeof...(P)>*);
template<class... T> using Both = Count<T..., char>;
int check(Size<1>*) { return 0; }
int check(Size<3>*) { return 0; }
int main() { Both<> a; Both<int, short> b; return check(&a) || check(&b); }
