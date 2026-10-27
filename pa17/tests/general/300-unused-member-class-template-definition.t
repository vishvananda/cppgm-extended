template<class T> struct Strip { using type = T; };
template<class T> struct Strip<T&> { using type = T; };
template<class Layer> struct Wrapper {
  using layer_type = typename Strip<Layer>::type;
  template<class U = int> struct Rebind {
    using other = typename layer_type::template Apply<U>::type;
    other value;
    explicit Rebind(other n) : value(n) {}
    other get() const { return value; }
  };
  char marker;
};
struct Missing {};
struct Valid { template<class U> struct Apply { using type = U; }; };
Wrapper<Missing> global{'g'};
int main() {
  Wrapper<Missing> blank{'b'};
  Wrapper<Valid>::Rebind<> integer{7};
  Wrapper<Valid&>::Rebind<char> character{'c'};
  return sizeof(blank) != 1 || blank.marker != 'b' || global.marker != 'g' ||
      integer.get() != 7 || character.get() != 'c';
}
