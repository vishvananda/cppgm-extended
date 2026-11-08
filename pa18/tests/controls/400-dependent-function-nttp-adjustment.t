// expect exit 0
// N3485 14.1p8, 14.3.2p5, 14.3.3p3: typed template parameter normalization.
void select(int) {} void select(float) {}
template<class T, T F, T G, bool equal = F == G> struct Pair { static int result() { return equal; } };
template<class T> int test() { return Pair<void(T), select, select>::result(); }
int main() { return test<int>() == 1 ? 0 : 1; }
