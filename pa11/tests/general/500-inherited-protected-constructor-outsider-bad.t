class Base { protected: Base(int) {} };
struct Derived:Base { using Base::Base; };
int main() { Derived value(7); }
