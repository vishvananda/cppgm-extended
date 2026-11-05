// P1825R0: a function-local static is not an implicitly movable entity;
// its conversion function must still receive an lvalue. expect exit 0
struct Result { int value; };
struct Source {
  operator Result() & { return { 1 }; }
  operator Result() && { return { 2 }; }
};
Result result() { static Source source; return source; }
int main() { return result().value == 1 ? 0 : 1; }
