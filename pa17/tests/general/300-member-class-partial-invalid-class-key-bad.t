template<class T> struct Outer {
  template<class U> struct Inner {};
  template<class U> union Inner<U*> { U value; };
};
