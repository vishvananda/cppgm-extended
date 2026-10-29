template<class T> struct Base { typedef T type; };
template<class T> int fixed(T value) {
    struct Local : Base<int> { };
    typedef typename Local::type Captured;
    Captured copy = value;
    return copy;
}
int main() { return fixed(7) + fixed(9L) - 16; }
