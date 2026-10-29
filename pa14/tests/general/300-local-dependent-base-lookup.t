template<class T> struct Base { typedef T type; };
template<class T> int direct(T value) {
    struct Local : Base<T> { };
    typedef typename Local::type Captured;
    Captured copy = value;
    return copy;
}
template<class T> int chain(T value) {
    struct First : Base<T> { };
    struct Second : First { };
    struct Local : Second { };
    typedef typename Local::type Captured;
    Captured copy = value;
    return copy;
}
int main() { return direct(7) + direct(9L) + chain(11) + chain(13L) - 40; }
