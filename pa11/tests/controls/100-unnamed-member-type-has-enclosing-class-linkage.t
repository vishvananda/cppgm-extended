// expect exit 0
struct Enclosing { struct { int value; } member; };
int external(decltype(Enclosing::member));
int read(decltype(Enclosing::member) value) { return external(value); }
