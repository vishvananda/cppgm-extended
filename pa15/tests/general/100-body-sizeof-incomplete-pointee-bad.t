template<unsigned long N> struct Size { enum { value = N }; };
template<class T> int width(T i) { return Size<sizeof(*i)>::value; }
struct Incomplete;
int main() { return width(static_cast<Incomplete*>(0)); }
