// expect reject
// N3485 5.2.10p11: a void prvalue does not supply storage for a reinterpret-cast reference.
struct Object {}; void source() {} int main() { auto&& object=reinterpret_cast<Object&&>(source()); }
