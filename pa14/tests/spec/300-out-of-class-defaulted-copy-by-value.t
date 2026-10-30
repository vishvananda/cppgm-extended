// N3485 focus: 8.4.2p4, 12.8p12. The constructor call and ABI agree.
template<class T> struct R {
  T* value;
  explicit R(T* input) : value(input) {}
  R(R const&);
};
template<class T> R<T>::R(R const&) = default;
int take(R<int> value) { return *value.value; }
int main() {
  int value = 7;
  R<int> input(&value);
  return take(input) != 7;
}
