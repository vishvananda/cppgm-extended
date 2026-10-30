typedef _Complex float C;
typedef C SC __attribute__((mode(SC)));
typedef C DC __attribute__((mode(DC)));
typedef C XC __attribute__((mode(XC)));
typedef C TC __attribute__((mode(__TC__)));
template<class T>struct A{typedef T Wide __attribute__((mode(TC)));typedef T Medium __attribute__((mode(DC)));};static_assert(__is_same(A<C>::Wide,TC), "");static_assert(__is_same(A<C>::Medium,DC), "");template<class T>struct B{typedef T Real __attribute__((mode(DI)));};static_assert(__is_same(B<unsigned>::Real,unsigned long), "");int main(){}
