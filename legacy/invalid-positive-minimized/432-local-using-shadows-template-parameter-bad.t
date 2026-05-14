// Minimized from pa22/tests/general/432-local-alias-shadowing-bound-type.t
template<class U>
struct S {
  static void f() {
    using U = int;
  }
};
