template<class T> struct Invalid { typedef typename T::missing type; };
struct Prefix {
  int value;
  Prefix& operator++() { ++value; return *this; }
  template<class T=Prefix> typename Invalid<T>::type operator++(int);
};
struct Postfix {
  int value;
  Postfix operator++(int) { Postfix old=*this; ++value; return old; }
  template<class T=Postfix> typename Invalid<T>::type operator++();
};
int main() { Prefix a={2}; ++a; Postfix b={3}; Postfix old=b++; return a.value!=3 || old.value!=3 || b.value!=4; }
