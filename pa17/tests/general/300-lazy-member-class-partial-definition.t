template<class Derived, class NullaryResult = void> struct Poly {
  template<class Sig> struct Result;
  template<class This> struct Result<This()> : Derived::template Impl<> {
    using type = typename Result::result_type;
  };
  NullaryResult operator()() const {
    Result<Derived const()> impl;
    return impl();
  }
};
struct Missing {};
Poly<Missing> unused;
struct Fun : Poly<Fun, int> {
  template<class T = void> struct Impl {
    using result_type = int;
    int operator()() const { return 7; }
  };
};
template<class... Outer> struct Packs {
  template<class Sig> struct Member;
  template<class... Inner> struct Member<void(Inner...)> {
    char value[sizeof...(Outer) + sizeof...(Inner) + 1];
    explicit Member(char n) : value{n} {}
    int size() const { return sizeof(value); }
    char first() const { return value[0]; }
  };
};
int main() {
  Packs<int, char>::Member<void(long)> one{'a'};
  Packs<long>::Member<void(int, char)> two{'b'};
  return Fun{}() != 7 || one.size() != 4 || two.size() != 4 ||
      one.first() != 'a' || two.first() != 'b' || sizeof(unused) != 1;
}
