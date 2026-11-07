// N3485 15.4p3-p4 and adopted CWG1330: preserve exception-specification agreement after complete-class lookup.
// expect reject
struct First { friend void g() noexcept(1 == 1); }; struct Second { friend void g() noexcept(false); };
