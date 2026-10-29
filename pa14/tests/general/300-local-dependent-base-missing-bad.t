template<class T> struct Base { };
template<class T> int capture(T value) {
    struct Local : Base<T> { };
    typedef typename Local::missing Captured;
    Captured copy = value;
    return copy;
}
int main() { return capture(7); }
