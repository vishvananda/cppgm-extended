struct Value { int number; };
template<class A> auto operator+(const A& a, const A& b) { return A{a.number + b.number}; }
template<class A> auto operator*(int factor, const A& a) { return A{factor * a.number}; }
Value result(Value a, Value b) { return 2 * (a + b); }
int main() { auto value = result(Value{3}, Value{4}); return value.number != 14; }
