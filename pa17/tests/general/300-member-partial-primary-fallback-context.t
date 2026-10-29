template<int N> struct Number { enum { value = N }; };
template<class T> struct Outer {
  typedef T nr_regions;
  typedef Number<nr_regions::value> Earlier;
  template<class State, int Dummy> struct Step {
    enum { value = Step<Number<State::value + 1>, Dummy>::value };
  };
  template<int Dummy> struct Step<Number<nr_regions::value>, Dummy> {
    enum { value = nr_regions::value };
    int read() const { return value; }
  };
  template<class State, class D> struct Other { enum { value = 0 }; };
  template<class D> struct Other<Number<nr_regions::value>, D> { enum { value = 1 }; };
};
template<class T> struct Forward {
  typedef T nr_regions;
  typedef Number<nr_regions::value> Earlier;
  template<class State, class D> struct Inner { enum { value = 0 }; };
  template<class D> struct Inner<Number<nr_regions::value>, D>;
};
template<class T> struct DefinedOutside {
  template<class State, class D> struct Inner { enum { value = 0 }; };
};
template<class T> template<class D> struct DefinedOutside<T>::Inner<T*, D> {
  enum { value = 5 };
  int read() const { return value; }
};
static_assert(Outer<Number<3>>::Step<Number<0>, 0>::value == 3, "recursive primary to selected stop");
static_assert(Outer<Number<3>>::Other<Number<4>, int>::value == 0, "unmatched inline partial");
static_assert(Outer<Number<3>>::Other<Number<3>, int>::value == 1, "matched inline partial");
static_assert(Forward<Number<3>>::Inner<Number<4>, int>::value == 0, "unmatched forward partial");
static_assert(DefinedOutside<int>::Inner<int*, char>::value == 5, "outside definition remains visible");
int main() {
  Outer<Number<3>>::Step<Number<3>, 0> end;
  DefinedOutside<int>::Inner<int*, char> outside;
  Forward<Number<3>>::Inner<Number<3>, int>* incomplete = 0;
  return end.read() != 3 || outside.read() != 5 || incomplete != 0;
}
