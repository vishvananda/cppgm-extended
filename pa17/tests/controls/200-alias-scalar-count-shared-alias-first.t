// CXX11-124: a known-length dependent scalar retains alias capture identity.
// N3485 5.3.3p5,14.5.7p2; expect exit 0
template<unsigned long N> struct Size {};
template<class... Q> using Count = Size<sizeof...(Q)>;
template<class... P> void prior(Size<sizeof...(P)>*);
template<class T> int inspect(Count<T>*) { return 0; }
int main() { Size<1> a; return inspect<int>(&a) || inspect<char>(&a); }
