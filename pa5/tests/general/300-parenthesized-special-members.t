// Parenthesized special-member names and complete declarators retain their nesting.
template<class Signature> struct Callable {
    Callable(const char*);
};
template<class T> struct Clamp : Callable<T(T, T, T)> {
    (Clamp)() : Callable<T(T, T, T)>("clamp") {}
};
struct C {
    typedef int argument;
    int value;
    ((C))() : value(7) {}
    (C(argument));
    ((~C())) {}
    ((operator int))() const { return value; }
    int (ordinary)(int n) { return n; }
};
((C::C))(argument n) : value(n) {}
struct D {
    typedef int argument;
    (D(argument));
    (operator int)() const;
};
((D::D(argument n))) {}
(D::operator int)() const { return 3; }
