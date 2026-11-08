// expect exit 0
// A fixed converted value does not replace per-specialization object identity.
struct Literal { constexpr operator int() const { return 4; } };
template<const Literal&> struct Identity { static int read() { return 0; } };
template<class T> struct Holder {
 static constexpr Literal object = Literal();
 typedef Identity<object> Id;
};
template<class T> constexpr Literal Holder<T>::object;
template<> struct Identity<Holder<int>::object> { static int read() { return 7; } };
int main() { return Holder<int>::Id::read() == 7 ? 0 : 1; }
