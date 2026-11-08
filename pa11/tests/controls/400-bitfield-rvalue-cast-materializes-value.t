// expect exit 0
// I13: N3485 5.2.9p3 converts a bit-field to a value before a compatible rvalue-reference cast.
struct Bits { signed int value : 4; };
int main() { Bits bits={-3}; int&& snapshot=static_cast<int&&>(bits.value); snapshot=5; return bits.value!=-3 || snapshot!=5; }
