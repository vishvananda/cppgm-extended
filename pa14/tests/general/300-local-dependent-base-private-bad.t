template<class T> class Base { typedef T type; };
template<class T> int capture(T value) {
    struct Local : Base<T> { };
    typedef typename Local::type Captured;
    Captured copy = value;
    return copy;
}
int main() { return capture(7); }
