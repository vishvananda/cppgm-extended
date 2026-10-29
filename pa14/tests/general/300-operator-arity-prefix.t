template<class T> struct Invalid { typedef typename T::missing type; };
struct Iterator {
  int value;
  Iterator& operator++() { ++value; return *this; }
  Iterator& operator--() { --value; return *this; }
  int operator+() const { return value; }
};
template<class T> typename Invalid<T>::type operator++(T&, int);
template<class T> typename Invalid<T>::type operator--(T&, int);
template<class T> typename Invalid<T>::type operator+(const T&, const T&);
int main() { Iterator i={4}; ++i; --i; return +i != 4; }
