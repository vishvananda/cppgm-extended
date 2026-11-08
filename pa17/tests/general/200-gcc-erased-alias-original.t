// GCC testsuite: gcc/testsuite/g++.dg/cpp0x/alias-decl-33.C
// CXX11-126: complete original compile-only translation under N3485 and adopted CWG1558.
// PR c++/51239
// { dg-require-effective-target c++11 }

template<class... x>
class list{};
template<class a, class... b>
using tail=list<b...>;
template <class...T>
void f(tail<T...>);		// { dg-error "alias" }

int main()
{
  f<int,int>({});
}
