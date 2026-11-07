// N3485 10.2p9: the same static function remains unambiguous across repeated bases.
// expect exit 0
int calls;
struct A { static void f() { ++calls; } void g(); };
struct B : private A { using A::f; using A::g; };
struct F : private A { using A::f; using A::g; };
struct E : B, F {};
int main() { E::f(); E object; object.f(); return calls==2 ? 0 : 1; }
