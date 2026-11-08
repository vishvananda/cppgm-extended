// expect reject
// N3485 5.19p2: a function-to-object pointer reinterpretation is not a core constant expression.
void function(int) {} constexpr void* pointer=(void*)&function; int main() { return 0; }
