template<class T> struct Friend;
struct Base {
    template<class> friend struct Friend;
protected:
    int value() const { return 7; }
};
struct Derived : Base {};
template<class T> struct Friend : protected T {
    static int get(T& object) { return object.value(); }
};
int main() {
    Derived object;
    return Friend<Derived>::get(object) != 7;
}
