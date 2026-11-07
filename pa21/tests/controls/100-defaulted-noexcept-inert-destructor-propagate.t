// N3485 12.4p8 and 15.5.1: preserve destruction and exception boundaries.
// Reduced from GCC g++.dg/cpp0x/defaulted43.C (PR53903).
// expect exit 0
struct T { ~T() noexcept(false) { throw 7; } };
struct A { ~A() noexcept(false); T t; };
A::~A() noexcept(false) = default;
int main() { try { A a; } catch (int v) { return v == 7 ? 0 : 1; } return 2; }
