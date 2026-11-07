// N3485 8.3p1, 7.3.1.2p2 and adopted CWG727: specialize where the primary may be defined.
// expect exit 0
namespace N { template<class T> int f(T*); }
using namespace N;
template<class T> int f(T);
template<> int f<int>(int) { return 0; }
int main() { return f(1); }
