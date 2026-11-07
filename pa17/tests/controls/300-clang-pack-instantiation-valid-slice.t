// expect exit 0
// Valid independent slice of LLVM Clang temp.variadic/deduction.cpp (lines 1-27).
// Upstream revision: 2078da43e25a4623cab2d0d60decddf709aaea28.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// RUN: %clang_cc1 -std=c++11 -fsyntax-only -verify %s
// expected-no-diagnostics

namespace DeductionForInstantiation {
  template<unsigned I, typename ...Types>
  struct X { };

  template<typename ...Types>
  void f0(X<sizeof...(Types), Types&...>) { }

  // No explicitly-specified arguments
  template void f0(X<0>);
  template void f0(X<1, int&>);
  template void f0(X<2, int&, short&>);

  // One explicitly-specified argument
  template void f0<float>(X<1, float&>);
  template void f0<double>(X<1, double&>);

  // Two explicitly-specialized arguments
  template void f0<char, unsigned char>(X<2, char&, unsigned char&>);
  template void f0<signed char, char>(X<2, signed char&, char&>);

  // FIXME: Extension of explicitly-specified arguments
  //  template void f0<short, int>(X<3, short&, int&, long&>);
}


int main() { return 0; }
