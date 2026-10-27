// expect reject
// N3485 focus: 9.3.1 [class.mfct.non-static]. An unevaluated call still
// needs an object; the rule allowing a data-member name does not allow calls.
struct Object { int value() const; };
typedef decltype(Object::value()) Invalid;
int main() { return 0; }
