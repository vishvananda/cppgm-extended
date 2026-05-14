// Minimized from pa22/tests/general/280-qualified-special-member-definitions.t
template<class T>
struct S {
  S();
};

template<class T>
S<T>::S() noexcept {}
