// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
struct S{static char value[31];int f(long value,int n=sizeof(value)){return n;}};
int main(){S s;return s.f(0)!=sizeof(long);}
