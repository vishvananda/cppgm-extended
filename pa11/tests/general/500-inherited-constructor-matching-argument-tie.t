struct Base { int value; Base(int n):value(n) {} };
struct Derived:Base { using Base::Base; Derived(int n,bool=false):Base(n+1) {} };
struct BetterConversion:Base { using Base::Base; BetterConversion(long n,bool=false):Base(n+2) {} };
int main() { Derived a(7); BetterConversion b(7); return a.value==8 && b.value==7?0:1; }
