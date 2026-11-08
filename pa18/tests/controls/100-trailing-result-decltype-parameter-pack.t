// expect exit 0
// N3485 7.1.6.2p4, 14.5.3: decltype selects one parameter type per pack element.
template<class... T> struct Result { typedef int type; };
template<class... T> auto count(T... values) -> typename Result<decltype(values)...>::type { return sizeof...(values); }
int main() { return count(1, short(2)) == 2 && count() == 0 ? 0 : 1; }
