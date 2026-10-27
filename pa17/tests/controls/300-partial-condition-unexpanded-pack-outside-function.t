// expect reject
// N3485 focus: 14.5.3 [temp.variadic]. A nested expansion does not expand
// another occurrence of the pack outside that function parameter list.
template<bool> struct Enable {};
template<> struct Enable<true> { typedef void type; };
template<class, class> struct Ready { static const bool value = true; };
template<class F, class = void> struct Pick;
template<class... T>
struct Pick<void(T...), typename Enable<Ready<T, void(T...)>::value>::type> {};
int main() { return 0; }
