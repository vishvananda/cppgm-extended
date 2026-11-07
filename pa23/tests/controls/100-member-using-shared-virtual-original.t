// Adapted from LLVM Project clang/test/CXX/class.derived/class.member.lookup/p9.cpp.
// Apache License v2.0 with LLVM Exceptions; see NOTICE.
// CWG39/N3485 10.2: identical declarations over one virtual base are unambiguous.
// expect exit 0
// RUN: %clang_cc1 -fsyntax-only -verify %s

namespace rdar8436162 {
  class ClsA {
  public:
    static void f();
    void g();
  };

  class ClsB : virtual private ClsA {
  public:
    using ClsA::f;
    using ClsA::g; // expected-note{{member found by ambiguous name lookup}}
  };

  class ClsF : virtual private ClsA {
  public:
    using ClsA::f;
    using ClsA::g; // expected-note{{member found by ambiguous name lookup}}
  };

  class ClsE : public ClsB, public ClsF {
    void test() { 
      f();
      g(); // expected-error{{member 'g' found in multiple base classes of different types}}
    }
  };
}

namespace rdar8436162 { void ClsA::f() {} void ClsA::g() {} }
int main() { return 0; }
