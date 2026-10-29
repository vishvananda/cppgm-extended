template<unsigned long N> struct Size { enum { value = N }; };
template<class T> int width(T i) {
    return Size<sizeof(*i)>::value;
}
template<class T> struct Iterator {
    T* pointer;
    T& operator*() const { return *pointer; }
};
int main() {
    char c = 0;
    long l = 0;
    Iterator<long> i = {&l};
    return width(&c) + width(&l) + width(i) - 17;
}
