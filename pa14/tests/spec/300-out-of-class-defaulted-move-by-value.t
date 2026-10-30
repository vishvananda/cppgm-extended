// N3485 focus: 8.4.2p4, 12.8p13. A later defaulted move is nontrivial too.
template<class T> struct R {
  T* value;
  explicit R(T* input) : value(input) {}
  R(R&&);
};
template<class T> R<T>::R(R&&) = default;
int take(R<int> value) { return *value.value; }
int main() {
  int value = 7;
  R<int> input(&value);
  return take(static_cast<R<int>&&>(input)) != 7;
}
