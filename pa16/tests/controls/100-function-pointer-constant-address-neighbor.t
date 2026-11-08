// expect exit 0
// N3485 5.19: forming a same-type function pointer constant is valid.
void function(int) {} constexpr void (*pointer)(int)=&function; static_assert(pointer==&function,"function identity"); int main() { pointer(7); return 0; }
