// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
template<class T> struct S{int f(T value,int n=sizeof(value)){return n;}};
int main(){S<long> s;return s.f(0)!=sizeof(long);}
