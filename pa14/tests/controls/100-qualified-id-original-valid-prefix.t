// N3485 template and namespace lookup: retain the independently valid qualified-id prefix.
// expect exit 0
// RUN: %clang_cc1 -fsyntax-only -verify %s

// PR5061
namespace a {
  template <typename T> class C {};
}
namespace b {
  template<typename T> void f0(a::C<T> &a0) { }
}


namespace test1 {
  int a = 0;
  template <class T> class Base { };
  template <class T> class Derived : public Base<T> {
    int foo() {
      return test1::a;
    }
  };
}

namespace test2 {
  class Impl {
  public:
    int foo();
  };
  template <class T> class Magic : public Impl {
    int foo() {
      return Impl::foo();
    }
  };
}

namespace PR6063 {
  template <typename T> void f(T, T);

  namespace detail
  {
    using PR6063::f;
  }

  template <typename T>
  void g(T a, T b)
  {
    detail::f(a, b);
  }
}

// The original valid prefix is retained above. Define its invoked function
// template here so the independently required runtime control has no missing definition.
namespace PR6063 { template<typename T> void f(T, T) {} }
// expect exit 0
int main() { a::C<int> value; b::f0(value); test1::Derived<int> derived; test2::Magic<int> magic; PR6063::g(1, 2); return test1::a; }
