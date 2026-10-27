// expect exit 0
// N3485 focus: 14.5.3 [temp.variadic], 14.5.5 [temp.class.spec].
// A function parameter pack inside a condition's type-id expands there.
template<bool> struct Enable {};
template<> struct Enable<true> { typedef void type; };
template<class> struct Ready { static const bool value = true; };
template<class F, class = void> struct Pick;
template<class... T>
struct Pick<void(T...), typename Enable<Ready<void(T...)>::value>::type> {
    static const int count = sizeof...(T);
};
template<class F, class = void> struct BySize;
template<class... T>
struct BySize<void(T...), typename Enable<(sizeof(void(*)(T...)) > 0)>::type> {
    static const int count = sizeof...(T);
};
int main() {
    return Pick<void(int, char, double)>::count != 3 || Pick<void()>::count != 0 ||
           BySize<void(int, char)>::count != 2 || BySize<void()>::count != 0;
}
