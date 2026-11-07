// N3485 8.3p1, 7.3.1.2p2 and adopted CWG727: specialize where the primary may be defined.
// expect reject
namespace N { namespace I { template<class T> void f(); } using I::f; }
template<> void N::f<int>();
