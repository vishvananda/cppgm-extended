struct Missing {};
template<class T> struct Outer {
  template<class U> struct Inner {};
  template<class U> struct Inner<U*> { using type = typename T::missing; };
};
Outer<Missing>::Inner<int*> object;
