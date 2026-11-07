// N3485 3.5p6, 5.19p2: block function declarations denote their matched namespace entity.
// GCC PR c++/111132. The declaration inside a constexpr body uses PA16
// documented extended body support; strict ISO11 neighbors are separate controls.
// PR c++/111132
// { dg-do compile { target c++11 } }

constexpr bool bar(void) {
    return true;
}

constexpr bool foo() {
    constexpr bool bar(void);
    return bar();
}

static_assert(foo(), "");
