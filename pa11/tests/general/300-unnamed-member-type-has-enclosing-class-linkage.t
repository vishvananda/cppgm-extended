// Compile-only: an external definition can be supplied by another translation unit.
struct Enclosing { struct { int value; } member; };
int external(decltype(Enclosing::member));
int read(decltype(Enclosing::member) value) { return external(value); }
