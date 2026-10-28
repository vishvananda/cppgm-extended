template<class> struct Friend;
struct Base {
    template<class> friend struct Friend;
protected:
    int value() const { return 7; }
};
struct Derived : Base {};
template<class T> struct Friend : Derived {
    template<class U> static auto get(U& object, int) -> decltype(object.value());
    template<class> static char get(...);
};
struct Nonfriend : Derived {
    template<class U> static auto get(U& object, int) -> decltype(object.value());
    template<class> static char get(...);
};
int main() {
    return sizeof(decltype(Friend<int>::get<Derived>(*(Derived*)0, 0))) != sizeof(int)
        || sizeof(decltype(Nonfriend::get<Derived>(*(Derived*)0, 0))) != sizeof(char);
}
