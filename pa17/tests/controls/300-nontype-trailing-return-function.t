// expect exit 0
// GCC auto23.C; N3485 8.3.5p2,14.1p8: resolve the trailing return, then adjust function to pointer.
template<auto Fn() -> int> struct FunctionParameter {};
int main() { return 0; }
