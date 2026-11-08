// expect reject
// I13: N3485 5.2.9p3 still requires reference compatibility before bit-field value conversion; do not remove volatile.
struct Bits { int value:4; }; int main() { volatile Bits bits={1}; static_cast<int&&>(bits.value); }
