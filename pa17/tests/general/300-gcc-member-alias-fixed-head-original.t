// GCC testsuite: gcc/testsuite/g++.dg/cpp0x/alias-decl-dr1286b.C
// CXX11-121: complete original compile-only translation; no link/runtime obligation is inferred.
// PR c++/60328
// { dg-require-effective-target c++11 }

template <class _T, class... _Rest>
struct Foo
{
  template <class _TT, class... _RR>
  using Bar = Foo<_TT, _RR...>;

  using Normal = Foo<_Rest...>;
  using Fail = Bar<_Rest...>;
};
