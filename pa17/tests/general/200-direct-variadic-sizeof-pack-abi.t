// CXX11-127: ordinary pack arguments retain their J...E grouping.
template<int...> struct Values {};
template<class... T> int measure(Values<sizeof...(T)>*) { return sizeof...(T); }
int main() { Values<2> value; return measure<int, char>(&value) != 2; }
