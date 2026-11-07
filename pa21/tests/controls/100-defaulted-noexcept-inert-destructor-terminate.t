// N3485 12.4p8 and 15.5.1: preserve destruction and exception boundaries.
// Reduced from GCC g++.dg/cpp0x/defaulted43.C (PR53903).
// expect exit 134: the course runtime terminates with status 134.
struct T { ~T() noexcept(false) { throw 7; } };
struct A { ~A() noexcept; T t; };
A::~A() noexcept = default;
int main() { A a; }
