// P1825R0 (P1155 DR): a deleted selected rvalue conversion is a hard
// error; it must not retry a valid lvalue conversion. expect reject
struct Result { int value; };
struct Source {
  operator Result() & { return { 1 }; }
  operator Result() && = delete;
};
Result result(Source source) { return source; }
int main() { return 0; }
