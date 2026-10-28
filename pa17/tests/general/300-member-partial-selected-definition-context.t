template<class T> struct Select { typedef T type; };
struct Two { enum { value = 2 }; typedef int result_type; };
struct Five { enum { value = 5 }; typedef long result_type; };
struct Missing { enum { value = 7 }; };
template<class T> struct Outer {
  typedef typename Select<T>::type nr_regions;
  template<int N, class D> struct Inner { enum { value = 0 }; };
  template<class D> struct Inner<nr_regions::value, D> {
    enum { value = 1 };
    typedef typename T::result_type type;
    int tag;
    explicit Inner(int v) : tag(v) {}
    int read() const { return tag + T::value; }
    template<class Z> int add() const { return tag + sizeof(Z); }
  };
  template<class U> struct Helper {
    template<int N, class D> struct Inner { enum { value = 0 }; };
    template<class D> struct Inner<nr_regions::value, D> {
      enum { value = 2 };
      int read() const { return T::value + sizeof(U) + sizeof(D); }
    };
  };
};
Outer<Missing> unused;
static_assert(Outer<Two>::Inner<2, char>::value == 1, "selected inline partial");
static_assert(Outer<Five>::Inner<5, int>::value == 1, "distinct outer context");
static_assert(Outer<Two>::Inner<5, int>::value == 0, "unmatched primary");
static_assert(Outer<Two>::Helper<int>::Inner<2, char>::value == 2, "nested selected partial");
static_assert(Outer<Five>::Helper<char>::Inner<5, int>::value == 2, "nested distinct context");
int main() {
  Outer<Two>::Inner<2, char> one(11);
  Outer<Five>::Inner<5, int> two(13);
  Outer<Two>::Helper<int>::Inner<2, char> nested;
  return one.read() != 13 || two.read() != 18 || one.add<char>() != 12 ||
    nested.read() != 7 || sizeof(unused) != 1;
}
