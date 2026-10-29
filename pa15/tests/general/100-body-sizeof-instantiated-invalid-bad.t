template<unsigned long N> struct Size { enum { value = N }; };
template<class T> int width(T i) { return Size<sizeof(*i)>::value; }
int main() { return width(1); }
