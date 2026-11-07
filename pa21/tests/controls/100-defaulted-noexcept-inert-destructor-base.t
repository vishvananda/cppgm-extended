// N3485 12.4p8 and 15.5.1: preserve destruction and exception boundaries.
// Reduced from GCC g++.dg/cpp0x/defaulted43.C (PR53903).
// expect exit 0
struct T { ~T() noexcept(false) {} };
struct A : T { ~A() noexcept; };
A::~A() noexcept = default;
int main() { A a; return 0; }
