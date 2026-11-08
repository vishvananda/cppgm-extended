// expect exit 0
// I13: A sole viable lvalue conversion result still initializes the ordinary copy constructor.
struct Value { int n; Value(int x) : n(x) {} }; Value stored(7);
struct Source { operator Value&() { return stored; } };
int main() { Source source; Value object(source); return object.n!=7; }
