template<class T> struct Invalid { typedef typename T::missing type; };
struct Iterator { int value; };
template<class T> typename Invalid<T>::type operator++(T&);
int main() { Iterator i={4}; ++i; }
