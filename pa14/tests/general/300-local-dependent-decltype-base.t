template<class T> struct Wrapper { typedef T type; };
template<class T> Wrapper<T> wrap(T&);
template<class T> int capture(T value) {
    struct Local : decltype(wrap(value)) { };
    typedef typename Local::type Captured;
    Captured copy = value;
    return copy;
}
int main() { return capture(7) + capture(9L) - 16; }
