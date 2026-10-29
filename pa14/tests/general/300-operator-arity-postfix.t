template<class T> struct Invalid { typedef typename T::missing type; };
struct Iterator {
  int value;
  Iterator operator++(int) { Iterator old=*this; ++value; return old; }
  Iterator operator--(int) { Iterator old=*this; --value; return old; }
  int operator+(int rhs) const { return value+rhs; }
};
template<class T> typename Invalid<T>::type operator++(T&);
template<class T> typename Invalid<T>::type operator--(T&);
template<class T> typename Invalid<T>::type operator+(const T&);
int main() { Iterator i={4}; Iterator old=i++; Iterator next=i--; return old.value!=4 || next.value!=5 || i+2!=6; }
