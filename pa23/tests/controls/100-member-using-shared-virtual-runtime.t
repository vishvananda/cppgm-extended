// N3485 10.2p3-p9,5.2.5p5: two public using paths share one virtual object.
// expect exit 0
int calls;
struct A { int value; A() : value(1) {} static void f() { ++calls; } void g() { ++value; } };
struct B : virtual private A { using A::f; using A::g; A* base_b() { return this; } };
struct F : virtual private A { using A::f; using A::g; A* base_f() { return this; } };
struct E : B, F { void run() { f(); g(); } };
int main() { E object; object.run(); object.g(); object.B::g(); object.F::g(); object.f(); A* b=object.base_b(); A* f=object.base_f(); return b==f && b->value==5 && calls==2 ? 0 : 1; }
