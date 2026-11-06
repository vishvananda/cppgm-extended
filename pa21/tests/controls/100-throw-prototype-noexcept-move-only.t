// CWG2549, adopted as a DR in February2025; N5009 CWG poll1/P3638R0.
// Retain C++11 two-stage overload resolution; this control tests eligibility.
// expect exit 0
struct Value { int tag; Value() : tag(1) {} Value(const Value&) = delete; Value(Value&&) : tag(3) {} };
void act(Value value) noexcept(noexcept(throw value));
int main() { return 0; }
