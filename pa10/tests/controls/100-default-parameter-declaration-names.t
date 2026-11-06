// CWG2082: parameter lookup in an unevaluated default argument. CXX11-039, independently reduced lookup controls.
// expect exit 0
char value[31];
int f(long value,int n=sizeof(value));
int f(long other,int out){return out;}
int main(){return f(0)!=sizeof(long);}
