// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
template<class T> int f(T value,int n=sizeof(value)){return n;}
int main(){return f(0L)!=sizeof(long);}
