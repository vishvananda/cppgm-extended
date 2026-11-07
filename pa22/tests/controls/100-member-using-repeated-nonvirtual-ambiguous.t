// N3485 5.2.5p5: replacing the shared virtual base by two nonvirtual bases is ambiguous.
// expect reject
struct A { static void f(); void g(); };
struct B : private A { using A::f; using A::g; };
struct F : private A { using A::f; using A::g; };
struct E : B, F { void test() { f(); g(); } };
