template<unsigned long N> struct Size { enum { value = N }; };
template<class T> int fixed(T) {
    int value = 0;
    return Size<sizeof(value) + sizeof(1)>::value;
}
int main() { return fixed(0) + fixed('x') - 16; }
