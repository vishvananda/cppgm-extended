typedef _Complex float C;
typedef C SC __attribute__((mode(SC)));
typedef C DC __attribute__((mode(DC)));
typedef C XC __attribute__((mode(XC)));
typedef C TC __attribute__((mode(__TC__)));
static_assert(__is_same(SC,_Complex float), "");static_assert(__is_same(DC,_Complex double), "");static_assert(__is_same(XC,_Complex long double), "");static_assert(sizeof(TC)==32&&alignof(TC)==16, "");static_assert(sizeof(__real__ *(TC*)0)==16, "");typedef C Mutable __attribute__((mode(DC)));typedef const Mutable Qual;static_assert(__is_same(Qual,const _Complex double), "");int main(){}
