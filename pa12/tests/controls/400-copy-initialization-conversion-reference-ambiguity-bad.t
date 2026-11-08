// expect reject
// I13: N3485 13.3: class copy/move candidate selection must retain competing conversion-function results, including deleted candidates.
struct Value { int n; Value(int x) : n(x) {} }; Value stored(5);
struct Source {
 operator Value() const { return Value(7); }
 operator Value&() { return stored; }
 operator const Value&() = delete;
 void run() { Value object(*this); }
};
int main() { Source source; source.run(); }
