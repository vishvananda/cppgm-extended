// LLVM Project contributors, Apache-2.0 WITH LLVM-exception; see doc/llvm-license.txt.
// Source: clang/test/CXX/except/except.spec/p13-friend.cpp
// Revision: 2078da43e25a4623cab2d0d60decddf709aaea28
// Adopted CWG1330: preserve the complete original valid translation, including both friend contexts.
// expect exit 0
// RUN: %clang_cc1 -fexceptions -fcxx-exceptions -fsyntax-only -verify %s

namespace N0 {
  void f() noexcept;
  void g() noexcept;

  struct A {
    friend void f() noexcept;
    friend void g() noexcept(x);

    static constexpr bool x = true;
  };
} // namespace N0

namespace N1 {
  void f() noexcept;
  void g();

  template<typename T>
  struct A {
    friend void f() noexcept;
    // FIXME: This error is emitted if no other errors occured (i.e. Sema::hasUncompilableErrorOccurred() is false).
    friend void g() noexcept(x); // expected-error {{no member 'x' in 'N1::A<int>'; it has not yet been instantiated}}
                                 // expected-note@-1 {{in instantiation of exception specification}}
    static constexpr bool x = false; // expected-note {{not-yet-instantiated member is declared here}}
  };

  template struct A<int>; // expected-note {{in instantiation of template class}}
} // namespace N1

int main() { return 0; }
