// CWG2549, adopted as a DR in February2025; N5009 CWG poll1/P3638R0.
// Retain C++11 two-stage overload resolution; this control tests eligibility.
// expect reject
struct Value { int tag; Value() : tag(1) {} Value(const Value&) : tag(2) {} Value(Value&&) = delete; };
template<class T> int probe(T&& value) { throw value; }
long probe(const Value&) { return 0; }
int main() { Value value; return probe(static_cast<Value&&>(value)); }
