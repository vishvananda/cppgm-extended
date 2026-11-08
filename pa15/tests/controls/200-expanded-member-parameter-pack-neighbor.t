// expect exit 0
// N3485 14.5.3p5: the function parameter pack is an expansion.
template<class... T> struct A { A(T*... values) {} };
int main() { int n = 0; A<int> a(&n); A<> empty; return 0; }
