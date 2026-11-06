// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
int f(int n=sizeof(value),long value=0){return n;}
int main(){return f()!=sizeof(::value);}
