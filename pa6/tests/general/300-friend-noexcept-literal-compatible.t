// N3485 15.4p3-p4 and adopted CWG1330: preserve exception-specification agreement after complete-class lookup.
void g() noexcept(false); struct A { friend void g() noexcept(false); };
