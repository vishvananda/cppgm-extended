// N3485 14.5.2, 14.7.3: a member template of an explicitly specialized class
// retains its own dependent result specialization across its definition.
// expect exit 0
namespace data {
template<class T> struct Value { T value; };
template<> struct Value<int> {
  template<class U> Value<U> get(U value) const;
  template<class U> Value<U> get(U* value, unsigned index) const;
};
int before_definition() { return Value<int>{}.get(41L).value; }
template<class U> Value<U> Value<int>::get(U value) const { return Value<U>{value}; }
template<class U> Value<U> Value<int>::get(U* value, unsigned index) const {
  return Value<U>{value[index]};
}
template<class T> struct Ordinary {
  template<class U> Value<U> get(U value) const;
};
template<class T> template<class U> Value<U> Ordinary<T>::get(U value) const {
  return Value<U>{value};
}
}
int main() {
  const data::Value<int> source{};
  long values[] = {17, 29};
  return data::before_definition() != 41 || source.get(values, 1).value != 29 ||
         data::Ordinary<char>{}.get(73L).value != 73;
}
