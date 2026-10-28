struct Object {
    int method() const;
    int overloaded_method(int) const;
    int overloaded_method(double) const;
    static int overloaded(int);
    static int overloaded(double);
};
template<class T> T& get();
template<class T> auto bound(int) -> decltype((get<T>().method), char{});
template<class> long bound(...);
template<class T> auto methods(int) -> decltype((get<T>().overloaded_method), char{});
template<class> long methods(...);
template<class T> auto discarded(int) -> decltype((void)T::overloaded, char{});
template<class> long discarded(...);
using WrongTarget = void (*)(char*);
template<class T> auto wrong_target(int) -> decltype(static_cast<WrongTarget>(T::overloaded), char{});
template<class> long wrong_target(...);
int take(int (*)(int));
template<class T> auto valid_target(int) -> decltype(take(T::overloaded), char{});
template<class> long valid_target(...);
int main() {
    return sizeof(decltype(bound<Object>(0))) != sizeof(long)
        || sizeof(decltype(methods<Object>(0))) != sizeof(long)
        || sizeof(decltype(discarded<Object>(0))) != sizeof(long)
        || sizeof(decltype(wrong_target<Object>(0))) != sizeof(long)
        || sizeof(decltype(valid_target<Object>(0))) != sizeof(char);
}
