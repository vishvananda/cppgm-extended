// expect reject
// N3485 14.1p8, 14.3.2p5: adjusting a dependent function type must retain its signature.
void select(float) {}
template<class T, T F> struct Holder {};
template<class T> Holder<void(T), select> test();
int main() { test<int>(); }
