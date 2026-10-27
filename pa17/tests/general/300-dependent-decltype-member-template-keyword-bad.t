struct Known { template<class U> U read() const; };
template<class T> T factory();
template<class T> int invalid() {
  decltype(factory<T>()) value;
  return value.read<int>();
}
