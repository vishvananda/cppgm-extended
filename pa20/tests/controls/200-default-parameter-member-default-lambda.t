// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
struct S{int f(int n=[](){static int v=2;return ++v;}()){return n;}};
int main(){S s;return s.f()!=3||s.f()!=4;}
