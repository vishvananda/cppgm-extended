// expect reject
// An invalid scalar construction in an instantiated function body remains
// a hard error rather than an immediate substitution failure.
template<class T> T make() { return T(1, 2); }
int main() { return make<int>(); }
