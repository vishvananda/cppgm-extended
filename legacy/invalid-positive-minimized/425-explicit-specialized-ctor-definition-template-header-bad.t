// Minimized from pa21/tests/general/425-explicit-specialization-out-of-class-ctor-replay.t
template<class T>
struct S {
  S();
};

template<>
struct S<int> {
  S();
};

template<>
S<int>::S() {}
