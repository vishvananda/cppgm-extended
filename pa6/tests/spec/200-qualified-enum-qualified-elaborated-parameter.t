// N3485 7.2p4, 7.1.6.3p2: qualified enum definitions and existing enum type uses remain valid.
// valid type declarations
struct A { enum class E { zero }; };
int f(enum A::E argument);
