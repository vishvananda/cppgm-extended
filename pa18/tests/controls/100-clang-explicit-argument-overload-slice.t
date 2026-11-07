// Clang temp.deduct.partial/p3.cpp ExplicitArgs slice; N3485 14.8.3. expect exit 0
// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

template<bool B>
struct A { };

constexpr A<false> a;
constexpr A<false> b;

constexpr int* x = nullptr;
constexpr short* y = nullptr;

namespace ExplicitArgs {
  template<typename T, typename U>
  constexpr int f(U) noexcept(noexcept(T())) {
    return 0;
  }

  template<typename T>
  constexpr int f(T*) noexcept {
    return 1;
  }


  static_assert(f<int>(1) == 0, "C++11 overload identity");
  static_assert(f<short>(y) == 1, "C++11 overload identity");

  template<typename T, typename U>
  constexpr int g(U*) noexcept(noexcept(T())) {
    return 3;
  }

  template<typename T>
  constexpr int g(T) noexcept {
    return 4;
  }


  static_assert(g<int>(y) == 3, "C++11 overload identity");
  static_assert(g<short>(1) == 4, "C++11 overload identity");
} // namespace ExplicitArgs


int main() { return 0; }
