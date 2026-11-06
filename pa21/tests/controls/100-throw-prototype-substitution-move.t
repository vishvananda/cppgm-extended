// CWG2549, adopted as a DR in February2025; N5009 CWG poll1/P3638R0.
// Retain C++11 two-stage overload resolution; this control tests eligibility.
// expect exit 0
struct Value { int tag; Value() : tag(1) {} Value(const Value&) = delete; Value(Value&&) : tag(3) {} };
template<class T> auto probe(T&& value) -> decltype(throw value, char()) { return 0; }
long probe(const Value&) { return 0; }
int main() { Value value; return sizeof(probe(static_cast<Value&&>(value))) != sizeof(char) || sizeof(probe(value)) != sizeof(long); }
