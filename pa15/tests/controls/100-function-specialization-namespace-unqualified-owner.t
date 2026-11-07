// N3485 8.3p1, 7.3.1.2p2 and adopted CWG727: specialize where the primary may be defined.
// expect exit 0
namespace N { template<class T> int f(); }
using namespace N;
namespace N { template<> int f<int>() { return 0; } }
int main() { return N::f<int>(); }
