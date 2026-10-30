// N3485 focus: 8.4.2p4, 12.8p12, 14.7.1p2. A later default is user-provided.
template<class T> struct R {
  T* value;
  explicit R(T* input) : value(input) {}
  R(R const&);
};
template<class T> R<T>::R(R const&) = default;
int main() {
  int value = 5;
  R<int> first(&value);
  R<int> second(first);
  *second.value += 2;
  return value != 7;
}
