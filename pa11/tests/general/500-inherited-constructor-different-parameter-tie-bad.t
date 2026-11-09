struct Base { Base(short) {} };
struct Derived:Base { using Base::Base; Derived(long,bool=false):Base(0) {} Derived(unsigned,bool=false):Base(0) {} };
int main() { Derived value(7); }
