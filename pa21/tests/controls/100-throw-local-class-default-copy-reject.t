// CWG2549, adopted as a DR in February2025; N5009 CWG poll1/P3638R0.
// Retain C++11 two-stage overload resolution; this control tests eligibility.
// expect reject
struct Value { int tag; Value() : tag(1) {} Value(const Value&) = delete; Value(Value&&) : tag(3) {} };
void act() { Value value; struct Local { void run() { try { struct Inner { void nested(int = decltype((throw value, 0))()); }; } catch (...) {} } }; Local instance; instance.run(); }
int main() { act(); }
