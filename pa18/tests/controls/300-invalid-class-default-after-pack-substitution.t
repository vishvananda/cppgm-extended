// expect exit 0
// Invalid defaults exposed by expanding a class argument pack fail the
// immediate substitution; explicitly supplying that argument avoids its default.
template<class T, class U = typename T::member> struct Box {};
struct Good { typedef int member; };
template<class... T> Box<T...> result();
template<class... T> auto choose(int) -> decltype(result<T...>(), int()) { return 1; }
template<class... T> int choose(...) { return 0; }
int main() {
    return choose<Good>(0) != 1 || choose<int>(0) != 0 ||
        choose<int, int>(0) != 1;
}
