// AUDIT-ID: MANGLE-CONV
// AUDIT-EXPECT: symbols
// AUDIT-OPT: O0
int storage=7;
struct X { template<class T> operator T(){return storage;} };
void force(){X x;const int i=x.operator const int();int& r=x.operator int&();r+=i;}
