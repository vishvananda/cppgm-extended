// N3485 15.4p3-p4 and adopted CWG1330: preserve exception-specification agreement after complete-class lookup.
// expect reject
void g() noexcept; struct A { friend void g() noexcept(x); static constexpr bool x=false; };
