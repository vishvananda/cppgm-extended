// expect reject
// N3485 5.2.10p11 requires a glvalue operand for reference reinterpretation.
struct Object { int n; }; int main() { auto&& object=reinterpret_cast<Object&&>(Object{7}); }
