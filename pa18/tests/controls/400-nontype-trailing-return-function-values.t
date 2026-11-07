// expect exit 0
// N3485 14.1p8,14.3.2: adjusted function, explicit pointer and reference arguments preserve the callable value.
int value() { return 7; }
template<auto Fn() -> int> struct Function { static int get() { return Fn(); } };
template<auto (*Fn)() -> int> struct Pointer { static int get() { return Fn(); } };
template<auto (&Fn)() -> int> struct Reference { static int get() { return Fn(); } };
int main() { return Function<value>::get() + Pointer<&value>::get() + Reference<value>::get() != 21; }
