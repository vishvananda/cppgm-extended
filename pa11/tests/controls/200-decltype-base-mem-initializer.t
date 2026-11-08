// expect exit 0
struct Base { int value; Base() : value(7) {} };
Base sample;
struct Derived : Base { Derived() : decltype(sample)() {} };
int main() { Derived d; return d.value == 7 ? 0 : 1; }
