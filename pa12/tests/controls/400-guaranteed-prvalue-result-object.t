struct Value { int n; Value(int x):n(x){} Value(const Value&)=delete; Value(Value&&)=delete; };
Value make() { return Value(42); }
int main() { Value value=make(); return value.n==42?0:1; }
