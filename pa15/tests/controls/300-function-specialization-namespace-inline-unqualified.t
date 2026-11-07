// N3485 8.3p1, 7.3.1.2p2 and adopted CWG727: specialize where the primary may be defined.
// expect exit 0
inline namespace I { template<class T> int f(); }
template<> int f<int>() { return 0; }
int main() { return I::f<int>(); }
