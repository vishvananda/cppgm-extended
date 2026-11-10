#include <cstdio>
// A virtual call needs its signature and slot; unused bodies need no emission.
int unused, base_constructions;
struct Unused {
    virtual int value() { ++unused; return 99; }
};
int external_dispatch(Unused* p) { return p->value(); }
struct Base {
    int payload;
    explicit Base(int n) : payload(n) { ++base_constructions; }
    virtual int value() { return payload; }
    virtual ~Base() {}
};
struct Derived : Base {
    Derived() : Base(17) {}
    int value() { return Base::value() + 1; }
};
int dispatch(Base* p) { return p->value(); }
int main() {
    Derived d;
    return dispatch(&d) != 18 || unused != 0 || base_constructions != 1;
}
