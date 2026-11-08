// expect exit 0
// N3485 5.2.10p11: a compatible class glvalue provides actual storage.
struct Object { int n; }; int main() { Object original={7}; Object&& view=reinterpret_cast<Object&&>(original); view.n=9; return original.n!=9; }
