// CXX11-127: a direct scalar expression in a variadic target retains J...E.
template<int...> struct Values {};
template<class T> int measure(Values<sizeof(T)>*) { return sizeof(T); }
int main() { Values<sizeof(int)> value; return measure<int>(&value) != sizeof(int); }
