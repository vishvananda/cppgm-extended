// expect reject
// N3485 9.8p4: constexpr does not permit a local class's static data member.
void f() { struct Local { static constexpr int value = 3; }; }
