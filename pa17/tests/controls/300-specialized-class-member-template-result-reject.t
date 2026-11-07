// N3485 14.5.2: an out-of-class definition must match the dependent result.
// expect reject
template<class T> struct Value {};
template<> struct Value<int> {
  template<class U> Value<U> get() const;
};
template<class U> Value<U*> Value<int>::get() const { return {}; }
