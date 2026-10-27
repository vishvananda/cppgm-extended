struct Known { template<class U> U read() const; };
template<class T> int invalid(T value) {
  auto copy = value;
  return copy.read<int>();
}
